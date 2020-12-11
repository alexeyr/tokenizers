#pragma once

#define DOCTEST_CONFIG_IMPLEMENT

#include "tokenizers-cpp/tokenizer.h"
#include "tokenizers-cpp/tests.rs.h"
#include "rust/cxx.h"

#include <doctest/doctest.h>

#include <vector>
#include <string>
#include <memory>

namespace huggingface {
namespace tokenizers {
static std::string data_dir_;

inline std::string data_file(std::string name) { return data_dir_ + name; }

#define COMPARE_CONTAINERS(actual, expected)                                  \
    do {                                                                      \
        auto actual_ = actual;                                                \
        auto expected_ = expected;                                            \
        REQUIRE(actual_.size() == expected_.size());                          \
        for (size_t i = 0; i < actual_.size(); i++) {                         \
            REQUIRE_MESSAGE(actual_[i] == expected_[i], "mismatch at " << i); \
        }                                                                     \
    } while (false)

TEST_SUITE("Normalizers") {
    TEST_CASE("Replace literal") {
        // clang-format off
        /*
        let original = "This is a ''test''";
        let normalized = "This is a \"test\"";

        let mut n = NormalizedString::from(original);
        Replace::new("''", "\"").unwrap().normalize(&mut n).unwrap();

        assert_eq!(&n.get(), &normalized);
        */
        // clang-format on
        NormalizedString n("This is a ''test''");
        Normalizer::replace_literal("''", "\"").normalize(n);
        CHECK(n.get_normalized() == "This is a \"test\"");
    }

    TEST_CASE("Replace regex") {
        // clang-format off
        /*
        let original = "This     is   a         test";
        let normalized = "This is a test";

        let mut n = NormalizedString::from(original);
        Replace::new(ReplacePattern::Regex(r"\s+".into()), ' ')
            .unwrap()
            .normalize(&mut n)
            .unwrap();

        assert_eq!(&n.get(), &normalized);
        */
        // clang-format on
        NormalizedString n("This     is   a         test");
        Normalizer::replace_regex(R"(\s+)", " ").normalize(n);
        CHECK(n.get_normalized() == "This is a test");
    }

    TEST_CASE("NFKC") {
        // clang-format off
        /*
        let original = "\u{fb01}".to_string();
        let normalized = "fi".to_string();
        let mut n = NormalizedString::from(original.clone());
        NFKC.normalize(&mut n).unwrap();

        assert_eq!(
            n,
            NormalizedString::new(original, normalized, vec![(0, 3), (0, 3)], 0)
        );

        assert_eq!(n.alignments_original(), vec![(0, 2), (0, 2), (0, 2)]);
        */
        // clang-format on
        NormalizedString n("ﬁ");
        Normalizer::nfkc().normalize(n);
        CHECK(n.get_normalized() == "fi");
    }

    // TODO make this a test case for Normalizer::sequence
    TEST_CASE("Sequence") {
        // clang-format off
        /*
        let original: String = "Cụ thể, bạn sẽ tham gia một nhóm các giám đốc điều hành tổ chức, các nhà lãnh đạo doanh nghiệp, các học giả, chuyên gia phát triển và tình nguyện viên riêng biệt trong lĩnh vực phi lợi nhuận…".to_string();
        let normalized = "cu the, ban se tham gia mot nhom cac giam đoc đieu hanh to chuc, cac nha lanh đao doanh nghiep, cac hoc gia, chuyen gia phat trien va tinh nguyen vien rieng biet trong linh vuc phi loi nhuan...".to_string();
        let mut n = NormalizedString::from(original);
        NFKD.normalize(&mut n).unwrap();
        StripAccents.normalize(&mut n).unwrap();
        Lowercase.normalize(&mut n).unwrap();
        assert_eq!(&n.get(), &normalized);
        */
        // clang-format on
        NormalizedString n(
            "Cụ thể, bạn sẽ tham gia một nhóm các giám đốc điều hành tổ chức, "
            "các nhà lãnh đạo doanh nghiệp, các học giả, chuyên gia phát triển "
            "và tình nguyện viên riêng biệt trong lĩnh vực phi lợi nhuận…");
        Normalizer::nfkd().normalize(n);
        Normalizer::strip_accents().normalize(n);
        Normalizer::lowercase().normalize(n);
        CHECK(
            n.get_normalized() ==
            "cu the, ban se tham gia mot nhom cac giam đoc đieu hanh to chuc, "
            "cac nha lanh đao doanh nghiep, cac hoc gia, chuyen gia phat trien "
            "va tinh nguyen vien rieng biet trong linh vuc phi loi nhuan...");
    }
}

TEST_SUITE("Pre-tokenizers") {
    TEST_CASE("BERT basic") {
        // clang-format off
        /*
        let pretok = BertPreTokenizer;
        let mut pretokenized: PreTokenizedString = "Hey friend!     How are you?!?".into();
        pretok.pre_tokenize(&mut pretokenized).unwrap();
        assert_eq!(
            pretokenized
                .get_splits(OffsetReferential::Original, OffsetType::Byte)
                .into_iter()
                .map(|(s, o, _)| (s, o))
                .collect::<Vec<_>>(),
            vec![
                ("Hey", (0, 3)),
                ("friend", (4, 10)),
                ("!", (10, 11)),
                ("How", (16, 19)),
                ("are", (20, 23)),
                ("you", (24, 27)),
                ("?", (27, 28)),
                ("!", (28, 29)),
                ("?", (29, 30)),
            ]
        );
        */
        // clang-format on
        PreTokenizer pretok = PreTokenizer::bert();
        PreTokenizedString pretokenized("Hey friend!     How are you?!?");
        pretok.pre_tokenize(pretokenized);
        auto splits = pretokenized.get_splits(OffsetReferential::Original,
                                              OffsetType::Byte);
        using StringAndOffsets = std::tuple<std::string, size_t, size_t>;
        std::vector<StringAndOffsets> expected{
            {"Hey", 0, 3},   {"friend", 4, 10}, {"!", 10, 11},
            {"How", 16, 19}, {"are", 20, 23},   {"you", 24, 27},
            {"?", 27, 28},   {"!", 28, 29},     {"?", 29, 30}};
        CHECK(splits.size() == expected.size());
        for (int i = 0; i < splits.size(); i++) {
            StringAndOffsets actual{splits[i].original, splits[i].start,
                                    splits[i].end};
            CHECK_MESSAGE(actual == expected[i], "mismatched splits at " << i);
        }
    }
}

TEST_SUITE("Tokenizers") {
    TEST_CASE("Bert") {
        Tokenizer tokenizer(
            WordPieceBuilder()
                .files(data_file("/bert-base-uncased-vocab.txt"))
                .unk_token("[UNK]")
                .max_input_chars_per_word(100)
                .build());

        tokenizer.with_normalizer(BertNormalizerBuilder().build())
            .with_pre_tokenizer(PreTokenizer::bert())
            .with_post_processor(
                PostProcessor::bert("[SEP]", 101, "[CLS]", 102));

        Encoding encoding = tokenizer.encode("My name is John", true);
        std::vector<rust::String> expected_tokens{"[CLS]", "my",   "name",
                                                  "is",    "john", "[SEP]"};
        COMPARE_CONTAINERS(expected_tokens, encoding.get_tokens());
        std::vector<uint32_t> expected_ids{102, 2026, 2171, 2003, 2198, 101};
        COMPARE_CONTAINERS(expected_ids, encoding.get_ids());
    }

    TEST_CASE("GPT2") {
        Tokenizer tokenizer(BpeBuilder()
                                .files(data_file("/gpt2-vocab.json"),
                                       data_file("/gpt2-merges.txt"))
                                .build());

        tokenizer.with_pre_tokenizer(PreTokenizer::byte_level(false, true));

        Encoding encoding = tokenizer.encode("My name is John", true);
        std::vector<rust::String> expected_tokens{"My", "Ġname", "Ġis",
                                                  "ĠJohn"};
        COMPARE_CONTAINERS(expected_tokens, encoding.get_tokens());
        std::vector<uint32_t> expected_ids{3666, 1438, 318, 1757};
        COMPARE_CONTAINERS(expected_ids, encoding.get_ids());
    }
}

inline bool run_tests(rust::Str data_dir) {
    doctest::Context context;
    data_dir_ = std::string(data_dir);
    return context.run() == 0;
}
}  // namespace tokenizers
}  // namespace huggingface
