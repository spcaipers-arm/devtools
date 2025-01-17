/*
 * Copyright (c) 2020-2021 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ProjMgrTestEnv.h"
#include "RteFsUtils.h"
#include "CrossPlatformUtils.h"

using namespace std;

string testinput_folder;
string testoutput_folder;
string testcmsispack_folder;
string schema_folder;

StdStreamRedirect::StdStreamRedirect() :
  m_outbuffer(""), m_cerrbuffer(""),
  m_stdoutStreamBuf(std::cout.rdbuf(m_outbuffer.rdbuf())),
  m_stdcerrStreamBuf(std::cerr.rdbuf(m_cerrbuffer.rdbuf()))
{
}

std::string StdStreamRedirect::GetOutString() {
  return m_outbuffer.str();
}

std::string StdStreamRedirect::GetErrorString() {
  return m_cerrbuffer.str();
}

StdStreamRedirect::~StdStreamRedirect() {
  // reverse redirect
  std::cout.rdbuf(m_stdoutStreamBuf);
  std::cerr.rdbuf(m_stdcerrStreamBuf);
}

void ProjMgrTestEnv::SetUp() {
  error_code ec;
  schema_folder        = string(TEST_FOLDER) + "../schemas";
  testinput_folder     = RteFsUtils::GetCurrentFolder() + "data";
  testoutput_folder    = RteFsUtils::GetCurrentFolder() + "output";
  testcmsispack_folder = string(CMAKE_SOURCE_DIR) + "test/packs";
  if (RteFsUtils::Exists(testoutput_folder)) {
    RteFsUtils::RemoveDir(testoutput_folder);
  }
  RteFsUtils::CreateDirectories(testoutput_folder);
  std::string testdata_folder = string(TEST_FOLDER) + "data";
  testdata_folder   = fs::canonical(testdata_folder).generic_string();
  testoutput_folder = fs::canonical(testoutput_folder).generic_string();
  schema_folder     = fs::canonical(schema_folder).generic_string();
  ASSERT_FALSE(testdata_folder.empty());
  ASSERT_FALSE(testoutput_folder.empty());
  ASSERT_FALSE(schema_folder.empty());

  // copy schemas
  std::string schemaDestDir = string(PROJMGRUNITTESTS_BIN_PATH) + "/../etc";
  if (RteFsUtils::Exists(schemaDestDir)) {
    RteFsUtils::RemoveDir(schemaDestDir);
  }
  RteFsUtils::CreateDirectories(schemaDestDir);
  fs::copy(fs::path(schema_folder), fs::path(schemaDestDir), fs::copy_options::recursive, ec);

  //copy test input data
  if (RteFsUtils::Exists(testinput_folder)) {
    RteFsUtils::RemoveDir(testinput_folder);
  }
  RteFsUtils::CreateDirectories(testinput_folder);
  fs::copy(fs::path(testdata_folder), fs::path(testinput_folder), fs::copy_options::recursive, ec);

  // copy local pack
  string srcPackPath, destPackPath;
  srcPackPath  = testcmsispack_folder + "/ARM/RteTest_DFP/0.2.0";
  destPackPath = testinput_folder + "/SolutionSpecificPack";
  if (RteFsUtils::Exists(destPackPath)) {
    RteFsUtils::RemoveDir(destPackPath);
  }
  RteFsUtils::CreateDirectories(destPackPath);
  fs::copy(fs::path(srcPackPath), fs::path(destPackPath), fs::copy_options::recursive, ec);

  // copy invalid packs
  string srcInvalidPacks, destInvalidPacks;
  srcInvalidPacks = testcmsispack_folder + "-invalid";
  destInvalidPacks = testinput_folder + "/InvalidPacks";
  if (RteFsUtils::Exists(destInvalidPacks)) {
    RteFsUtils::RemoveDir(destInvalidPacks);
  }
  RteFsUtils::CreateDirectories(destInvalidPacks);
  fs::copy(fs::path(srcInvalidPacks), fs::path(destInvalidPacks), fs::copy_options::recursive, ec);

  CrossPlatformUtils::SetEnv("CMSIS_PACK_ROOT", testcmsispack_folder);

  // create dummy cmsis compiler root
  const string& testdir = testinput_folder + "/TestToolchains";
  RteFsUtils::CreateDirectories(testdir);
  RteFsUtils::CreateFile(testdir + "/AC5.5.6.7.cmake", "");
  RteFsUtils::CreateFile(testdir + "/AC6.6.18.0.cmake", "");
  RteFsUtils::CreateFile(testdir + "/GCC.11.2.1.cmake", "");
  RteFsUtils::CreateFile(testdir + "/IAR.8.50.6.cmake", "");
  CrossPlatformUtils::SetEnv("CMSIS_COMPILER_ROOT", testdir);
}

void ProjMgrTestEnv::TearDown() {
  // Reserved
}

int main(int argc, char **argv) {
  try {
    testing::InitGoogleTest(&argc, argv);
    ::testing::AddGlobalTestEnvironment(new ProjMgrTestEnv);
    return RUN_ALL_TESTS();
  }
  catch (testing::internal::GoogleTestFailureException const& e) {
    cout << "runtime_error: " << e.what();
    return 2;
  }
  catch (...) {
    cout << "non-standard exception";
    return 2;
  }
}
