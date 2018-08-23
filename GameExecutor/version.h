#pragma once

#ifndef HUDSON_BUILD
    #define FILEVER          1,0,0,0
    #define PRODUCTVER       FILEVER
    #define STRFILEVER       "1,0,0,0"
    #define STRPRODUCTVER    STRFILEVER

    #define COMPANYNAME      "ProtocolOne"
    #define FILEDESCRIPTION  "Developer version of ProtocolOne GameExecutor library"
    #define INTERNALNAME     "GameExecutor"
    #define LEGALCOPYRIGHT   "Copyright(c) 2010 - 2012"

    #ifdef DEBUG 
        #define ORIGINALFILENAME "GameExecutorX86d.dll"
    #else
        #define ORIGINALFILENAME "GameExecutorX86.dll"
    #endif

    #define PRODUCTNAME      "ProtocolOne GameExecutor library"
#else
    #define FILEVER          $$MAJOR$$,$$MINOR$$,$$HUDSON_BUILD$$
    #define PRODUCTVER       FILEVER
    #define STRFILEVER       "$$MAJOR$$,$$MINOR$$,$$HUDSON_BUILD$$,$$GIT_REVISION$$"
    #define STRPRODUCTVER    STRFILEVER

    #define COMPANYNAME      "$$COMPANYNAME$$"
    #define FILEDESCRIPTION  "$$FILEDESCRIPTION$$"
    #define INTERNALNAME     "$$INTERNALNAME$$"
    #define LEGALCOPYRIGHT   "$$LEGALCOPYRIGHT$$"
    #define ORIGINALFILENAME "$$FILENAME$$"
    #define PRODUCTNAME      "$$PRODUCTNAME$$"
#endif

