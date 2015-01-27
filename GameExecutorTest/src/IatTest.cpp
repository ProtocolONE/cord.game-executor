/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2015, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/
#include <Windows.h>

#include <Iat/HelperTypes.h>
#include <Iat/FunctionTable.h>
#include <Iat/ModuledUtils.h>
#include <Iat/PeUtils.h>
#include <Iat/WinVersion.h>

#include <gtest/gtest.h> 

#include <vector>
#include <string>
#include <algorithm>

class IatTest : public ::testing::Test 
{
protected:
  virtual void SetUp() 
  {
  }

  virtual void TearDown() 
  {
  }

  void initIternal()
  {
    GGS::GameExecutor::Iat::initInnerApi();
    GGS::GameExecutor::Iat::createInnerTable(this->_alias, this->_dllApi);

    GGS::GameExecutor::Iat::DllHolder::iterator it = this->_dllApi.find(L"KERNEL32.DLL");
    bool test1 = it != this->_dllApi.end();

    ASSERT_EQ(true, test1);

    GGS::GameExecutor::Iat::WinType wType = GGS::GameExecutor::Iat::getWinType();

    size_t funAddress = it->second.second;

    switch(wType)
    {
    case GGS::GameExecutor::Iat::WIN_XP_TYPE:
      ASSERT_EQ(0, funAddress);
      break;
    case GGS::GameExecutor::Iat::WIN_VISTA:
      ASSERT_NE(0, funAddress);
      break;
    case GGS::GameExecutor::Iat::WIN_NEXT:
      ASSERT_EQ(0, funAddress);
      break;
    case GGS::GameExecutor::Iat::WIN_UNDEF:
      //Nothing to do
      break;
    }
  }

  std::vector<std::wstring> _loadedModules;
  std::vector<std::wstring> _temporary;

  GGS::GameExecutor::Iat::DllHolder _dllApi;
  GGS::GameExecutor::Iat::Alias _alias;
};

TEST_F(IatTest, testModulesLoading)
{
  bool result = GGS::GameExecutor::Iat::getLoadedModules(this->_loadedModules);

  ASSERT_EQ(true, result);
  ASSERT_NE(0, this->_loadedModules.size());

  bool foundKernel32 = false;
  for (auto it = this->_loadedModules.begin(); it != this->_loadedModules.end(); ++it) {
    std::wstring forMatch = *it;
    transform(forMatch.begin(), forMatch.end(), forMatch.begin(), towupper);
    if (forMatch == L"KERNEL32.DLL") {
      foundKernel32 = true;
      break;
    }
  }
  ASSERT_EQ(true, foundKernel32);
}

TEST_F(IatTest, testRvaToIat)
{
  HMODULE krnlHandle = LoadLibraryW(L"kernel32.dll");
  size_t rva = GGS::GameExecutor::Iat::getIatRva(krnlHandle);

  ASSERT_NE(0, rva);
}

TEST_F(IatTest, testCreateTable)
{
  initIternal();
}

TEST_F(IatTest, testCopyVictim)
{
  initIternal();
  bool result = GGS::GameExecutor::Iat::copyVictimsSavePointer(this->_alias, this->_dllApi, this->_temporary);
  ASSERT_EQ(true, result);
  ASSERT_EQ(true, this->_temporary.size() != 0);

  for (auto it = this->_dllApi.begin(); it != this->_dllApi.end(); ++it) {
    //Check pointers to patched api
    ASSERT_NE(0, it->second.second);
  }
}