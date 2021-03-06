#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "sat.h"
#include "sat_util.h"
#include "list.h"

#include "satlib/construct.h"

Satellite* currentSat = NULL;
SatPart* currentPart = NULL;
SatPart* lastPart = NULL;

#define ARGS "(name, x, y)"
int satlib_sat_new(lua_State *L) {
	if(	lua_type(L, 1) == LUA_TSTRING &&
		lua_type(L, 2) == LUA_TNUMBER &&
		lua_type(L, 3) == LUA_TNUMBER
	) {
		// get args
		const char* name = lua_tostring(L, 1);
		int x = (int)lua_tonumber(L, 2);
		int y = (int)lua_tonumber(L, 3);

		// construct the satellite
		Satellite* newsat = calloc(1, sizeof(Satellite));

		newsat->name = calloc(strlen(name), sizeof(char));
		strcpy(newsat->name, name);

		newsat->x = x;
		newsat->y = y;

		SatPart* rootPart = calloc(1, sizeof(SatPart));
		rootPart->info = info_by_name("mainframe");
		newsat->parts = calloc(1, sizeof(Node));
		newsat->parts->data = rootPart;

		// add the satellite
		list_add(satellites, newsat);

		// update construction state
		currentSat = newsat;
		currentPart = rootPart;
	} else {
		return luaL_error(L, "%s: argument should be %s", __func__, ARGS);
	}

	return 0;
}
#undef ARGS

#define ARGS "(satellite name)"
int satlib_sat_select(lua_State *L) {
	if(lua_type(L, 1) == LUA_TSTRING) {
		// get args
		const char* name = lua_tostring(L, 1);

		// get satellite
		Satellite* sat = sat_by_name((char*)name);

		if(sat == NULL) 
			return luaL_error(L, "%s: there is no satellite named %s", __func__, name);

		// update construction state
		currentSat = sat;
	} else {
		return luaL_error(L, "%s: argument should be %s", __func__, ARGS);
	}

	return 0;
}
#undef ARGS

#define ARGS "none"
int satlib_sat_part_last(lua_State *L) {
	// update construction state
	SatPart* swapPart = currentPart;
	currentPart = lastPart;
	lastPart = swapPart;

	return 0;
}
#undef ARGS

#define ARGS "(x, y, direction)"
int satlib_sat_part_go(lua_State *L) {
	if(	lua_type(L, 1) == LUA_TNUMBER &&
		lua_type(L, 2) == LUA_TNUMBER &&
		lua_type(L, 3) == LUA_TNUMBER
	) {
		// get args
		int x = lua_tonumber(L, 1);
		int y = lua_tonumber(L, 2);
		Dir dir = lua_tonumber(L, 3);

		// get connector
		Connector* conn = connector_by_sig(currentPart->info, x, y, dir);

		// look for a part connected to currentPart by that connector
		Node* current = currentSat->parts;
		SatPart* candidate;
		while(current != NULL) {
			candidate = (SatPart*)current->data;
			if(candidate->connection->parent == conn) break;
			current = current->next;
		}

		if(current == NULL) candidate = NULL;
		if(current == NULL) 
			return luaL_error(L, "%s: there is no part connected to connector { %d, %d, %d }", __func__, x, y, dir);

		// update construction state
		lastPart = currentPart;
		currentPart = candidate;
	} else {
		return luaL_error(L, "%s: argument should be %s", __func__, ARGS);
	}

	return 0;
}
#undef ARGS

#define ARGS "(part name, child x, child y, child direction, rotation, parent x, parent y, parent direction)"
int satlib_sat_part_add(lua_State *L) {
	if(	lua_type(L, 1) == LUA_TSTRING &&
		lua_type(L, 2) == LUA_TNUMBER &&
		lua_type(L, 3) == LUA_TNUMBER &&
		lua_type(L, 4) == LUA_TNUMBER &&
		lua_type(L, 5) == LUA_TNUMBER &&
		lua_type(L, 6) == LUA_TNUMBER &&
		lua_type(L, 7) == LUA_TNUMBER &&
		lua_type(L, 8) == LUA_TNUMBER
	) {
		// get args
		const char* partName = lua_tostring(L, 1);
		int cx = (int)lua_tonumber(L, 2);
		int cy = (int)lua_tonumber(L, 3);
		Dir cdir = (int)lua_tonumber(L, 4);
		Rot crot = (int)lua_tonumber(L, 5);

		int px = (int)lua_tonumber(L, 6);
		int py = (int)lua_tonumber(L, 7);
		Dir pdir = (int)lua_tonumber(L, 8);

		// create the new part
		SatPartInfo* info = info_by_name((char*)partName);

		if(info == NULL)
			return luaL_error(L, "%s: the part type \"%s\" does not exist.", __func__, partName);

		Connector* connParent = connector_by_sig(currentPart->info, px, py, pdir);
		Connector* connChild = connector_by_sig(info, cx, cy, cdir);

		if(connParent == NULL || connChild == NULL)
			return luaL_error(L, "%s: connector(s) do(es) not exist.", __func__);

		if(!connectable(connParent, currentPart->rotation, connChild, crot))
			return luaL_error(L, "%s: incompatible directions.", __func__);

		SatPart* newpart = sat_part_add(currentSat, info, currentPart, crot, connParent, connChild);

		// update construction state
		lastPart = currentPart;
		currentPart = newpart;
	} else {
		return luaL_error(L, "%s: argument should be %s", __func__, ARGS);
	}

	return 0;
}
#undef ARGS
