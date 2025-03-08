// Downloaded from https://developer.x-plane.com/code-sample/vr-window-sample/
#include "XPLMDisplay.h"    // for window creation and manipulation
#include "XPLMGraphics.h"   // for window drawing
#include "XPLMDataAccess.h" // for the VR dataref
#include "XPLMPlugin.h"     // for XPLM_MSG_ENTERED_VR and XPLM_MSG_EXITING_VR message
#include "XPLMMenus.h"
#include <string.h>
#include <stdio.h> // for sprintf()
#if IBM
	#include <windows.h>
#endif
#if LIN
#include <GL/gl.h>
#endif
#if __GNUC__ && APL
#include <OpenGL/gl.h>
#endif
#if __GNUC__ && IBM
#include <GL/gl.h>
#endif

#ifndef XPLM301
	#error This is made to be compiled against the XPLM301 SDK
#endif

static XPLMWindowID	g_window;
static XPLMMenuID g_menu_id;

void				create_window();
void				get_default_window_pos(int position_lbrt[4]);
void				exit_vr_mode();
void				draw(XPLMWindowID in_window_id, void * in_refcon);
void				draw_button(const char * text, float in_out_lbrt[4] /* you set the left and bottom, we'll set the right and top */);
int					handle_mouse(XPLMWindowID in_window_id, int x, int y, int is_down, void * in_refcon);
void				handle_menu(void * mRef, void * iRef);

int					dummy_mouse_handler(XPLMWindowID in_window_id, int x, int y, int is_down, void * in_refcon) { return 0; }
XPLMCursorStatus	dummy_cursor_status_handler(XPLMWindowID in_window_id, int x, int y, void * in_refcon) { return xplm_CursorDefault; }
int					dummy_wheel_handler(XPLMWindowID in_window_id, int x, int y, int wheel, int clicks, void * in_refcon) { return 0; }
void				dummy_key_handler(XPLMWindowID in_window_id, char key, XPLMKeyFlags flags, char virtual_key, void * in_refcon, int losing_focus) { }

XPLMDataRef g_vr_dref;
static float g_vr_button_lbrt[4] = {0, 0, 0, 0}; // left, bottom, right, top
static float g_size_button_lbrt[4] = {0, 0, 0, 0}; // left, bottom, right, top
static int g_last_mouse_xy[2] = {0, 0};

static int	coord_in_rect(float x, float y, float * bounds_lbrt)  { return ((x >= bounds_lbrt[0]) && (x < bounds_lbrt[2]) && (y < bounds_lbrt[3]) && (y >= bounds_lbrt[1])); }


PLUGIN_API int XPluginStart(
						char *		outName,
						char *		outSig,
						char *		outDesc)
{
	strcpy(outName, "VRSamplePlugin");
	strcpy(outSig, "xpsdk.examples.vrsampleplugin");
	strcpy(outDesc, "A test plug-in that demonstrates moving a window into VR.");

	create_window();
	
	g_vr_dref = XPLMFindDataRef("sim/graphics/VR/enabled");
	
	g_menu_id = XPLMCreateMenu("VR Sample", NULL, 0, handle_menu, NULL);
	XPLMAppendMenuItem(g_menu_id, "Toggle Into/Out of VR", (void *)"cmd_vr", 1);
	
	// If this dataref is for some reason not available,
	// we won't be able to move the window to VR anyway,
	// so go ahead an disable us!
	return g_vr_dref != NULL;
}

PLUGIN_API void	XPluginStop(void)
{
	// Since we created the window, we'll be good citizens and clean it up
	XPLMDestroyWindow(g_window);
	g_window = NULL;
}

PLUGIN_API void XPluginDisable(void) { }
PLUGIN_API int  XPluginEnable(void)  { return 1; }
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void * inParam)
{
	if(inFrom == XPLM_PLUGIN_XPLANE && inMsg == XPLM_MSG_ENTERED_VR)
	{
		XPLMSetWindowPositioningMode(g_window, xplm_WindowVR, -1);
	}
	else if(inFrom == XPLM_PLUGIN_XPLANE && inMsg == XPLM_MSG_EXITING_VR && XPLMWindowIsInVR(g_window))
	{
		exit_vr_mode();
	}
}

void create_window()
{
	int position_lbrt[4];
	get_default_window_pos(position_lbrt);
	
	XPLMCreateWindow_t params = {};
	params.structSize = sizeof(params);
	params.left = position_lbrt[0];
	params.bottom = position_lbrt[1];
	params.right = position_lbrt[2];
	params.top = position_lbrt[3];
	params.visible = 1;
	params.drawWindowFunc = draw;
	params.handleMouseClickFunc = handle_mouse;
	params.handleRightClickFunc = dummy_mouse_handler;
	params.handleMouseWheelFunc = dummy_wheel_handler;
	params.handleKeyFunc = dummy_key_handler;
	params.handleCursorFunc = dummy_cursor_status_handler;
	params.refcon = NULL;
	params.layer = xplm_WindowLayerFloatingWindows;
	params.decorateAsFloatingWindow = xplm_WindowDecorationRoundRectangle;
	
	g_window = XPLMCreateWindowEx(&params);
	
	const int vr_is_enabled = XPLMGetDatai(g_vr_dref);
	XPLMSetWindowPositioningMode(g_window, vr_is_enabled ? xplm_WindowVR : xplm_WindowPositionFree, -1);
	
	XPLMSetWindowResizingLimits(g_window, 200, 200, 500, 500); // Limit resizing our window: maintain a minimum width/height of 200 boxels and a max width/height of 500
	XPLMSetWindowTitle(g_window, "Sample VR Window");
}

void get_default_window_pos(int position_lbrt[4])
{
	// We're not guaranteed that the main monitor's lower left is at (0, 0)...
	// we'll need to query for the global desktop bounds!
	int global_desktop_bounds[4]; // left, bottom, right, top
	XPLMGetScreenBoundsGlobal(&global_desktop_bounds[0], &global_desktop_bounds[3], &global_desktop_bounds[2], &global_desktop_bounds[1]);
	position_lbrt[0] = global_desktop_bounds[0] + 50;
	position_lbrt[1] = global_desktop_bounds[1] + 150;
	position_lbrt[2] = global_desktop_bounds[0] + 350;
	position_lbrt[3] = global_desktop_bounds[1] + 450;
}

void draw(XPLMWindowID in_window_id, void * in_refcon)
{
	XPLMSetGraphicsState(
			0 /* no fog */,
			0 /* 0 texture units */,
			0 /* no lighting */,
			0 /* no alpha testing */,
			1 /* do alpha blend */,
			1 /* do depth testing */,
			0 /* no depth writing */
	);

	int l, t, r, b;
	XPLMGetWindowGeometry(in_window_id, &l, &t, &r, &b);

	float col_white[] = {1.0, 1.0, 1.0};
	int char_height;
	XPLMGetFontDimensions(xplmFont_Proportional, NULL, &char_height, NULL);
	
	const int vr_is_enabled = XPLMGetDatai(g_vr_dref);
	if(vr_is_enabled)
	{
		// Position the button in the upper left of the window (sized to fit the button text)
		g_vr_button_lbrt[0] = l + 10;
		g_vr_button_lbrt[1] = t - char_height * 1.5;
		draw_button("Toggle VR", g_vr_button_lbrt);
		
		g_size_button_lbrt[0] = g_vr_button_lbrt[2] + 10;
		g_size_button_lbrt[1] = g_vr_button_lbrt[1];
		draw_button("Toggle Size", g_size_button_lbrt);
	}
	else // Draw some text to say VR isn't available
	{
		char * line1 = "VR must be enabled before you";
		char * line2 = "can toggle this window into VR.";
		XPLMDrawString(col_white, l, t - 2 * char_height, line1, NULL, xplmFont_Proportional);
		XPLMDrawString(col_white, l, t - 4 * char_height, line2, NULL, xplmFont_Proportional);
	}
}

void draw_button(const char * text, float in_out_lbrt[4] /* you set the left and bottom, we'll set the right and top */)
{
	// We draw our rudimentary button boxes based on the height of the button text
	int char_height;
	XPLMGetFontDimensions(xplmFont_Proportional, NULL, &char_height, NULL);
	
	in_out_lbrt[2] = in_out_lbrt[0] + XPLMMeasureString(xplmFont_Proportional, text, strlen(text)); // *just* wide enough to fit the button text
	in_out_lbrt[3] = in_out_lbrt[1] + (1.25f * char_height); // a bit taller than the button text
	
	// Draw the box around our rudimentary button
	float green[] = {0.0, 1.0, 0.0, 1.0};
	glColor4fv(green);
	glBegin(GL_LINE_LOOP);
	{
		glVertex2i(in_out_lbrt[0], in_out_lbrt[3]);
		glVertex2i(in_out_lbrt[2], in_out_lbrt[3]);
		glVertex2i(in_out_lbrt[2], in_out_lbrt[1]);
		glVertex2i(in_out_lbrt[0], in_out_lbrt[1]);
	}
	glEnd();
	
	// Draw the button text (pop in/pop out)
	float col_white[] = {1.0, 1.0, 1.0};
	XPLMDrawString(col_white, in_out_lbrt[0], in_out_lbrt[1] + 4, (char *)text, NULL, xplmFont_Proportional);
}

int	handle_mouse(XPLMWindowID in_window_id, int x, int y, XPLMMouseStatus mouse_status, void * in_refcon)
{
	if(mouse_status == xplm_MouseDown)
	{
		g_last_mouse_xy[0] = x;
		g_last_mouse_xy[1] = y;
		if(!XPLMIsWindowInFront(in_window_id))
		{
			XPLMBringWindowToFront(in_window_id);
		}
		else
		{
			const int vr_is_enabled = XPLMGetDatai(g_vr_dref);
			if(vr_is_enabled && coord_in_rect(x, y, g_vr_button_lbrt)) // user clicked the "toggle VR mode" button
			{
				if(XPLMWindowIsInVR(in_window_id))
				{
					exit_vr_mode();
				}
				else
				{
					XPLMSetWindowPositioningMode(in_window_id, xplm_WindowVR, 0);
				}
			}
			else if(vr_is_enabled && coord_in_rect(x, y, g_size_button_lbrt)) // user clicked "toggle size" btn
			{
				int width, height;
				XPLMGetWindowGeometryVR(in_window_id, &width, &height);
				width = width > 300 ? 300 : 500;
				height = height > 300 ? 300 : 500;
				XPLMSetWindowGeometryVR(in_window_id, width, height);
			}
		}
	}
	return 1;
}

void handle_menu(void * mRef, void * iRef)
{
	if(!strcmp((char *)iRef, "cmd_vr"))
	{
		if(XPLMWindowIsInVR(g_window))
		{
			exit_vr_mode();
		}
		else
		{
			XPLMSetWindowPositioningMode(g_window, xplm_WindowVR, 0);
		}
	}
}

void exit_vr_mode()
{
	XPLMSetWindowPositioningMode(g_window, xplm_WindowPositionFree, 0);
	// We need to also reposition the window; it doesn't get automatically moved back to the main monitor!
	int position_lbrt[4];
	get_default_window_pos(position_lbrt);
	XPLMSetWindowGeometry(g_window, position_lbrt[0], position_lbrt[3], position_lbrt[2], position_lbrt[1]);
}


