#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

int seisu;
double jissu;
char mojiretu[300];

// gcc -o setting setting.c -llua
/*
  http://nvnote.com/c-configfile-lua/
   もし、luaが入っていない場合
   sudo apt-get install lua
   sudo apt-get install lua-devel
*/

/*
   https://qiita.com/kb10uy/items/976a52f687bcb7745fc7
   上記でもダメな場合
   wget http://www.lua.org/ftp/lua-5.2.3.tar.gz
   tar xvf lua-5.2.3.tar.gz
   cd lua-5.2.3
   make linux && sudo make install
*/

/* ------------------------------------------------------------ */
/* lua変数を取得しスタックに格納する。取得できなければエラーを出力し終了*/
void Get_lua_variable(lua_State *L, const char *variable_name){
  lua_pop(L, 1);                                /* スタックから変数をポップする *\//\*一番上のスタックをPOP */
  lua_getglobal(L,variable_name);		/* 変数を取得しスタックに格納 */
  if(!lua_isstring(L,-1)){			/* "-" でスタックの一番上を示す*/
    printf("変数 \"%s\" は正しく取得できませんでした\n",variable_name);
    exit(1);
  }
}

/* ------------------------------------------------------------ */
/* Lua変数をint型で返却する */
int Get_lua_integer(lua_State *L, const char *variable_name){
  Get_lua_variable(L,variable_name);	/* lua変数をスタックに格納する */
  return lua_tointeger(L,-1);		/* スタックの一番上の値を取得 */
}

/* ------------------------------------------------------------ */
/* Lua変数をdouble型で返却する */
double Get_lua_number(lua_State *L, const char *variable_name){
  Get_lua_variable(L,variable_name);
  return lua_tonumber(L,-1);
}

/* ------------------------------------------------------------ */
/* Lua変数をchar *型で返却する */
char *Get_lua_string(lua_State *L, const char *variable_name){
  Get_lua_variable(L,variable_name);
  return (char *)lua_tostring(L,-1);
}

/* ------------------------------------------------------------ */
/* luaファイルから変数を取得し格納する*/
void Set_lua_variable(char *config_file_name){
  lua_State* L=luaL_newstate();	/*lua オブジェクトの生成*/
  luaL_openlibs(L);		/*標準ライブラリの読み込み*/
  luaL_dofile(L,config_file_name);	/*Luaファイルの評価*/

  /* 設定値の取得 */
  seisu = Get_lua_integer(L,"seisu");
  jissu = Get_lua_number(L,"jissu");
  strcpy(mojiretu,Get_lua_string(L,"mojiretu"));

  lua_close(L);				/*  Luaオブジェクトを解放 */
}

/* ------------------------------------------------------------ */
/* main文 */
int main(void){
  Set_lua_variable("config.lua");

  printf("sisu : %d\n",seisu);
  printf("jissu: %lf\n",jissu);
  printf("mojiretu : %s\n",mojiretu);
  return 0;
}
