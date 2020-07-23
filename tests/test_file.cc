#include <cstdio>
#include <fstream>
#include <memory>
#include <filesystem>

#include <gtest/gtest.h>

#define DEFER_1(x, y) x##y
#define DEFER_2(x, y) DEFER_1(x, y)
#define DEFER_3(x)    DEFER_2(x, __COUNTER__)
#define defer(code)   std::shared_ptr<void> DEFER_3(_defer_)(nullptr, [&](...){code;})


#include "file/hash.h"

const char testdata[] =
"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vestibulum "
"vestibulum blandit lacus, vitae placerat dolor tempor vitae. Praesent cursus "
"sagittis turpis. Phasellus hendrerit purus euismod pulvinar sagittis. Mauris "
"aliquam nisl lacus, pretium efficitur dui rhoncus vel. Duis semper augue eget "
"neque iaculis, et rutrum nulla ultricies. Donec volutpat nisi in risus rutrum "
"molestie. Curabitur sapien lacus, facilisis id libero rutrum, ultricies "
"maximus tortor. Phasellus dignissim condimentum tempus. Donec malesuada magna "
"vel posuere rhoncus. Suspendisse non molestie urna. Pellentesque suscipit "
"rhoncus facilisis. Pellentesque habitant morbi tristique senectus et netus et "
"malesuada fames ac turpis egestas.";
FileHash testdata_hash = 0x99434ED1D2F22B2Aull;

TEST(FileHash, buf) {
  FileHash hash = FileHash_from_buf(testdata, strlen(testdata));
  EXPECT_EQ(hash, testdata_hash);

  char *s_hash = FileHash_to_buf(hash, nullptr);
  FileHash hash_ = FileHash_from_string(s_hash);
  g_free(s_hash);
  EXPECT_EQ(hash, hash_);
}


#include "file/localindex.h"
#include "file/entry.h"

TEST(LocalFileIndex, index) {
  struct LocalFileIndex index;
  ASSERT_EQ(LocalFileIndex_init(&index), 0);
  defer(LocalFileIndex_destroy(&index));

  const char path[] = "data/sample";
  std::ofstream ofs(path);
  ofs << testdata;
  ofs.close();
  defer(remove(path));

  GError *error = NULL;
  struct FileEntry *entry = LocalFileIndex_get(&index, path, &error);
  ASSERT_NE(entry, nullptr) << error->message;
  EXPECT_EQ(entry->__anon.hash, testdata_hash);
  const char *abspath = entry->__anon.path;
  EXPECT_EQ(abspath[0], '/');
  EXPECT_EQ(strncmp(abspath + strlen(abspath) - strlen(path), path, strlen(path)), 0);
}


#include "file/cache.h"

TEST(Cache, cache) {
  const char cache_dir[] = "data/cache";

  struct Cache cache;
  ASSERT_EQ(Cache_init(&cache, cache_dir, false), 0);
  defer(std::filesystem::remove_all(cache_dir));
  defer(Cache_destroy(&cache));

  GError *error = NULL;
  struct CacheEntry *entry = Cache_index_buf(&cache, testdata, strlen(testdata), &error);
  ASSERT_NE(entry, nullptr) << (error ? error->message : "");
  defer(CacheEntry_unref(entry));
  EXPECT_EQ(entry->__anon.__anon.hash, testdata_hash);
  const char cache_path[] = "9/9/4/99434ED1D2F22B2A";
  EXPECT_STREQ(entry->__anon.__anon.path, cache_path);

  struct CacheEntry *entry_ = Cache_index_buf(&cache, testdata, strlen(testdata), &error);
  EXPECT_EQ(entry, entry_);
  CacheEntry_unref(entry);
}
