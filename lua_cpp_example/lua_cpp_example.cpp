#include <map>
#include <string>

extern "C" {

#include "lua.h"
#include "lauxlib.h"

/*
	Compatiblity with 5.1.
	The functions in this section were copied from Lua 5.2's auxilary library.
*/
#if LUA_VERSION_NUM < 502
#define LIBNAME "cppexample"

#define luaL_setfuncs(L, l, n) luaL_register(L, NULL, (l))
#define luaL_newlib(L, l) luaL_register(L, LIBNAME, (l))

static void luaL_setmetatable (lua_State *L, const char *tname) {
  luaL_getmetatable(L, tname);
  lua_setmetatable(L, -2);
}

#endif

} // extern "C"

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT extern
#endif

using namespace std;

// The name of the doublemap type. This is used in luaL_checkudata and luaL_newmetatable.
#define DOUBLEMAP "doublemap"

// Give our map type a proper name
typedef std::map<std::string, double> doublemap;

// Gets the doublemap argument at index i. Returns a doublemap pointer.
#define getdoublemap(L, i) (* (doublemap **) luaL_checkudata(L, i, DOUBLEMAP) )

// Push an existing doublemap onto the Lua stack as a userdata.
static void push_doublemap(lua_State *L, doublemap *M){
	doublemap **dm = (doublemap **)lua_newuserdata(L, sizeof(doublemap **)); // dm will store a pointer to a doublemap pointer
	*dm = M; // dm now points to the doublemap pointer argument
	luaL_setmetatable(L, DOUBLEMAP); // Set the new userdata's metatable to the doublemap metatable
}

// __index metamethod. If the key exists in the doublemap, returns it; otherwise returns nil.
static int doublemap_index(lua_State *L){
	doublemap *M = getdoublemap(L, 1);
	
	// Lua works with C char strings but doublemap uses C++ strings
	size_t len = -1;
	const char *key = luaL_checklstring(L, 2, &len);
	std::string keystr(key, len);
	
	doublemap::iterator it = M->find(keystr);

	if ( it == M->end() ) {
		lua_pushnil(L);
	} else {
		double val = it->second;
		lua_pushnumber(L, val);
	}

	return 1;
}

// __newindex metamethod. Sets the key to the provided value.
static int doublemap_newindex(lua_State *L){
	doublemap *M = getdoublemap(L, 1);

	size_t len = -1;
	const char *key = luaL_checklstring(L, 2, &len);
	std::string keystr(key, len);

	double val = luaL_checknumber(L, 3);
	(*M)[keystr] = val;

	return 0;
}

// Garbage collection finaliser metamethod. This should safely delete the object in some way.
static int doublemap_gc(lua_State *L){
	doublemap *M = getdoublemap(L, 1);
	// delete M; // Only use this if you can be sure the doublemap was created with new.
	return 0;
}

// Create a new doublemap and push it onto the stack as a userdata
static int doublemap_new(lua_State *L){
	doublemap *M = new doublemap;
	push_doublemap(L, M);
	return 1;
}

static struct luaL_Reg metafuncs[] = {
	{"__index", doublemap_index},
	{"__newindex", doublemap_newindex},
	{"__gc", doublemap_gc},
	{NULL, NULL}
};

static struct luaL_Reg libfuncs[] = {
	{"new", doublemap_new},
	{NULL, NULL}
};

extern "C" {

EXPORT int luaopen_cppexample(lua_State *L){
	luaL_newmetatable(L, DOUBLEMAP);
	luaL_setfuncs(L, metafuncs, 0);

	luaL_newlib(L, libfuncs);
	
	doublemap *m1 = new doublemap;
	(*m1)["test1"] = 0.012;
	(*m1)["test2"] = 1.234;
	push_doublemap(L, m1);
	lua_setglobal(L, "mymap");

	doublemap *m2 = new doublemap;
	(*m2)["test1"] = 6.777;
	(*m2)["test2"] = 5.666;
	push_doublemap(L, m2);
	lua_setglobal(L, "mymap2");

	return 1;
}

}