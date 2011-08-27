
#include "HippoGui.h"
#include <core/debug/Assert.h>
#include <graphics/HippoImageLoader.h>
#include <graphics/HippoWindow.h>
#include <string.h>
#include <stdio.h>
#include <sys/syslimits.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

///

enum
{
	MAX_CONTROLS = 1024,
	MAX_IMAGES = 1024,
};

HippoGuiState g_hippoGuiState;
uint32_t s_controlId;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct GuiPlacementInfo
{
	enum GuiPlacementState
	{
		PLACEMENTSTATE_NONE,
		PLACEMENTSTATE_HORIZONAL,
		PLACEMENTSTATE_VERTICAL,
	};

	enum GuiPlacementState state;
	int x;
	int y;

} GuiPlacementInfo;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static GuiPlacementInfo g_placementInfo;
HippoControlInfo g_controls[MAX_CONTROLS];

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HippoGui_beginHorizontalStackPanelXY(int x, int y)
{
	g_placementInfo.state = PLACEMENTSTATE_HORIZONAL;
	g_placementInfo.x = x;
	g_placementInfo.y = y;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HippoGui_begin()
{
	s_controlId = 1;
	g_controls[0].type = DRAWTYPE_NONE;
	memset(&g_placementInfo, 0, sizeof(GuiPlacementInfo));
}

struct LoadedImages
{
	char filename[PATH_MAX][MAX_IMAGES];
	HippoImage image[MAX_IMAGES];
	uint32_t count;
} g_loadedImages;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HippoGui_reset()
{
	memset(&g_loadedImages, 0, sizeof(struct LoadedImages)); // not really needed but good when debugging
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static HippoImage* loadImage(const char* filename)
{
	uint32_t index = g_loadedImages.count;

	HIPPO_ASSERT(filename);

	// TODO: Hash filenames?

	for (uint i = 0; i != index; ++i)
	{
		if (!strcmp(filename, g_loadedImages.filename[i]))
			return &g_loadedImages.image[i];
	}

	// Load new image

	if (!HippoImageLoader_loadFile(&g_loadedImages.image[index], filename))
	{
		printf("Unable to load %s\n", filename);
		return 0;
	}	
	
	strcpy(g_loadedImages.filename[index], filename); g_loadedImages.count++;

	return &g_loadedImages.image[index];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool HippoGui_regionHit(const HippoControlInfo* control)
{
	//printf("%d %d\n", g_hippoGuiState.mousex, g_hippoGuiState.mousey);  
	//printf("%d %d %d %d\n", control->x, control->y, control->width, control->height);

	if (g_hippoGuiState.mousex < control->x ||
		g_hippoGuiState.mousey < control->y ||
		g_hippoGuiState.mousex >= control->x + control->width ||
		g_hippoGuiState.mousey >= control->y + control->height)
	{
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HippoGui_drawBorder(uint32_t color0, uint32_t color1, int x, int y, int w, int h)
{
	HippoGui_fill(color0, x, y, 1, h);
	HippoGui_fill(color0, x, y, w, 1);
	HippoGui_fill(color1, x + w, y, 1, h);
	HippoGui_fill(color1, x, y + h, w, 1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HippoGui_fill(uint32_t color, int x, int y, int w, int h)
{
	uint32_t controlId = 0;
	HippoControlInfo* control = 0; 

	// Setup the control
	controlId = s_controlId++;
	control = &g_controls[controlId];
	control->type = DRAWTYPE_FILL;
	control->x = x;
	control->y = y;
	control->width = w;
	control->height = h;
	control->color = color;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t genericImageControl(const char* filename)
{
	uint32_t controlId = 0;
	HippoControlInfo* control = 0; 
	HippoImage* image = loadImage(filename);

	if (!image)
		return ~0;

	// Setup the control
	
	controlId = s_controlId++;
	control = &g_controls[controlId];
	control->type = DRAWTYPE_IMAGE;
	control->x = g_placementInfo.x;
	control->y = g_placementInfo.y;
	control->width = image->width;
	control->height = image->height;
	control->imageData = image; 

	switch (g_placementInfo.state)
	{
		case PLACEMENTSTATE_NONE :
		{
			break;
		}

		case PLACEMENTSTATE_HORIZONAL :
		{
			g_placementInfo.x += image->width;
			break;
		}

		case PLACEMENTSTATE_VERTICAL :
		{
			g_placementInfo.y += image->height;
			break;
		}
	}

	return controlId;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HippoGui_staticImage(const char* filename)
{
	genericImageControl(filename);	
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool HippoGui_buttonImage(const char* filename)
{
	HippoControlInfo* control;
	uint32_t controlId = 0;
	
	controlId = genericImageControl(filename);

	if (controlId == ~0)
		return false;

	control = &g_controls[controlId];

	if (HippoGui_regionHit(control))
	{
		g_hippoGuiState.hotItem = controlId;
    	if (g_hippoGuiState.activeItem == 0 && g_hippoGuiState.mouseDown)
      		g_hippoGuiState.activeItem = controlId;
	}

	if (g_hippoGuiState.mouseDown == 0 && g_hippoGuiState.hotItem == controlId && g_hippoGuiState.activeItem == controlId)
		return true;

	return false;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HippoGui_end()
{
	if (g_hippoGuiState.mouseDown == 0)
	{
		g_hippoGuiState.activeItem = 0;
	}
	else
	{
		if (g_hippoGuiState.activeItem == 0)
			g_hippoGuiState.activeItem = -1;
	}

	HippoWindow_refresh();
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int luaButtonImage(lua_State* luaState)
{
	const char* path = luaL_checkstring(luaState, 1);
	int ret = HippoGui_buttonImage(path);
	if (ret)
		lua_pushnumber(luaState, 1);
	else
		lua_pushnil(luaState);


	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int luaFill(lua_State* luaState)
{
	uint32_t color = (uint32_t)luaL_checklong(luaState, 1);
	int x = luaL_checkint(luaState, 2);
	int y = luaL_checkint(luaState, 3);
	int w = luaL_checkint(luaState, 4);
	int h = luaL_checkint(luaState, 5);
	HippoGui_fill(color, x, y, w, h);

	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int luaDrawBorder(lua_State* luaState)
{
	uint32_t color0 = (uint32_t)luaL_checklong(luaState, 1);
	uint32_t color1 = (uint32_t)luaL_checklong(luaState, 2);
	int x = luaL_checkint(luaState, 3);
	int y = luaL_checkint(luaState, 4);
	int w = luaL_checkint(luaState, 5);
	int h = luaL_checkint(luaState, 6);
	HippoGui_drawBorder(color0, color1, x, y, w, h);

	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int luaBeginHorizontalStackPanelXY(lua_State* luaState)
{
	int x = luaL_checkint(luaState, 1);
	int y = luaL_checkint(luaState, 2);
	HippoGui_beginHorizontalStackPanelXY(x, y);

	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int luaStaticImage(lua_State* luaState)
{
	const char* name = luaL_checkstring(luaState, 1);
	HippoGui_staticImage(name);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const luaL_Reg uiLib[] =
{
	{ "buttonImage", luaButtonImage },
	{ "fill", luaFill },
	{ "drawBorder", luaDrawBorder },
	{ "beginHorizontalStackPanelXY", luaBeginHorizontalStackPanelXY },
	{ "staticImage", luaStaticImage },
	{ 0, 0 },
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int registerUiLib(lua_State* state)
{
	luaL_newlib(state, uiLib);
	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HippoGui_registerLuaFunctions(struct lua_State* luaState)
{
	luaL_requiref(luaState, "hippo_ui", registerUiLib, 1);
    lua_pop(luaState, 1);  
}


