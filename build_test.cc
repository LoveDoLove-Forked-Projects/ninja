#include "build.h"

#include <gtest/gtest.h>

TEST(Subprocess, Ls) {
  Subprocess ls;
  string err;
  EXPECT_TRUE(ls.Start("ls /", &err));
  ASSERT_EQ("", err);

  // Pretend we discovered that stdout was ready for writing.
  ls.OnFDReady(ls.stdout_.fd_);

  EXPECT_TRUE(ls.Finish(&err));
  ASSERT_EQ("", err);
  EXPECT_NE("", ls.stdout_.buf_);
  EXPECT_EQ("", ls.stderr_.buf_);
}

TEST(Subprocess, BadCommand) {
  Subprocess subproc;
  string err;
  EXPECT_TRUE(subproc.Start("ninja_no_such_command", &err));
  ASSERT_EQ("", err);

  // Pretend we discovered that stderr was ready for writing.
  subproc.OnFDReady(subproc.stderr_.fd_);

  EXPECT_FALSE(subproc.Finish(&err));
  EXPECT_NE("", err);
  EXPECT_EQ("", subproc.stdout_.buf_);
  EXPECT_NE("", subproc.stderr_.buf_);
}

TEST(SubprocessSet, Single) {
  SubprocessSet subprocs;
  Subprocess* ls = new Subprocess;
  string err;
  EXPECT_TRUE(ls->Start("ls /", &err));
  ASSERT_EQ("", err);
  subprocs.Add(ls);

  while (!ls->done()) {
    subprocs.DoWork(&err);
    ASSERT_EQ("", err);
  }
  ASSERT_NE("", ls->stdout_.buf_);
}

TEST(SubprocessSet, Multi) {
  SubprocessSet subprocs;
  Subprocess* processes[3];
  const char* kCommands[3] = {
    "ls /",
    "whoami",
    "pwd",
  };

  string err;
  for (int i = 0; i < 3; ++i) {
    processes[i] = new Subprocess;
    EXPECT_TRUE(processes[i]->Start(kCommands[i], &err));
    ASSERT_EQ("", err);
    subprocs.Add(processes[i]);
  }

  for (int i = 0; i < 3; ++i) {
    ASSERT_FALSE(processes[i]->done());
    ASSERT_EQ("", processes[i]->stdout_.buf_);
    ASSERT_EQ("", processes[i]->stderr_.buf_);
  }

  while (!processes[0]->done() || !processes[1]->done() ||
         !processes[2]->done()) {
    subprocs.DoWork(&err);
    ASSERT_EQ("", err);
  }

  for (int i = 0; i < 3; ++i) {
    ASSERT_NE("", processes[i]->stdout_.buf_);
    ASSERT_EQ("", processes[i]->stderr_.buf_);
    delete processes[i];
  }
}
