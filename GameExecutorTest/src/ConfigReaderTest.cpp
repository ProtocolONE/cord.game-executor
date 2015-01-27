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

#include <Common/ConfigReader.h>
#include <gtest/gtest.h>

#include <string>
#include <vector>

class ConfigTest : public ::testing::Test 
{
protected:
  virtual void SetUp() 
  {
    this->_serviceId = L"300012010000000000";
    //Creating shared memory
    this->_writer.setPid(GetCurrentProcessId());
    //Setting flags
    this->_writer.setCommandLineFlag(true);
    this->_writer.setSpeedHackFlag(true);
    this->_writer.setServiceId(this->_serviceId);

    //Setting strings
    std::vector<std::wstring> strHolder;
    strHolder.push_back(L"test1");
    strHolder.push_back(L"test2");
    strHolder.push_back(L"test3");
    strHolder.push_back(L"test4");
    this->_writer.setStrings(strHolder);
    //Writing
    this->_writer.write();

    //Creating reader
    this->_reader.open();
    this->_reader.read();
  }

  virtual void TearDown() 
  {
    this->_writer.close();
    this->_reader.close();
  }

  GGS::GameExecutorHelper::ConfigReader _reader;
  GGS::GameExecutorHelper::ConfigReader _writer;
  std::wstring _serviceId;
};

TEST_F(ConfigTest, testCommandLineTest)
{
  EXPECT_EQ(true, _reader.isCommandLineEnabled());
}

TEST_F(ConfigTest, testSpeedHackFlag)
{
  EXPECT_EQ(true, _reader.isSpeedHackEnabled());
}

TEST_F(ConfigTest, testNumberOfStrings)
{
  EXPECT_EQ(4, _reader.getStrings().size());
}

TEST_F(ConfigTest, testMatchStrings)
{
  EXPECT_EQ(L"test1", _reader.getStrings()[0]);
  EXPECT_EQ(L"test2", _reader.getStrings()[1]);
  EXPECT_EQ(L"test3", _reader.getStrings()[2]);
  EXPECT_EQ(L"test4", _reader.getStrings()[3]);
}

TEST_F(ConfigTest, testServiceId)
{
  EXPECT_EQ(this->_serviceId, _reader.serviceId());
}