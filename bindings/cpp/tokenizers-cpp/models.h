#pragma once

#include "tokenizers-cpp/common.h"
#include "tokenizers-cpp/models.rs.h"

#include <nonstd/optional.hpp>

#include <string>
#include <unordered_map>

namespace huggingface {
namespace tokenizers {

struct BpeBuilder;

struct Model {
    HFT_FFI_WRAPPER(Model);

public:
    // static HFT_RESULT(Model) bpe(BpeBuilder& builder) {
    //     return builder.build();
    // }

    HFT_RESULT(rust::Vec<Token>) tokenize(nonstd::string_view sequence) {
        HFT_TRY(rust::Vec<Token>,
                ffi::tokenize(*inner_, string_view_to_str(sequence)).tokens);
    }

    nonstd::optional<uint32_t> token_to_id(nonstd::string_view token) {
        return HFT_OPTION(
            ffi::token_to_id_model(*inner_, string_view_to_str(token)));
    }

    nonstd::optional<std::string> id_to_token(uint32_t id) {
        ffi::OptionString opt_token(ffi::id_to_token_model(*inner_, id));
        return opt_token.has_value
                   ? nonstd::make_optional(std::string(opt_token.value))
                   : nonstd::nullopt;
    }

    size_t get_vocab_size() { return ffi::get_vocab_size_model(*inner_); }

    std::unordered_map<std::string, uint32_t> get_vocab() {
        rust::Vec<ffi::KVStringU32> entries(ffi::get_vocab_model(*inner_));
        std::unordered_map<std::string, uint32_t> vocab;
        for (auto& entry : entries) {
            vocab[std::string(entry.key)] = entry.value;
        }
        return vocab;
    }

    HFT_RESULT(rust::Vec<rust::String>) save(nonstd::string_view folder) {
        HFT_TRY(rust::Vec<rust::String>,
                ffi::save(*inner_, string_view_to_str(folder), false, {}));
    }

    HFT_RESULT(rust::Vec<rust::String>)
    save(nonstd::string_view folder, nonstd::string_view prefix) {
        HFT_TRY(rust::Vec<rust::String>,
                ffi::save(*inner_, string_view_to_str(folder), true,
                          string_view_to_str(prefix)));
    }
};

struct BpeBuilder {
    HFT_FFI_WRAPPER(BpeBuilder);

public:
    BpeBuilder() : inner_(ffi::bpe_builder()){};

    HFT_RESULT(Model) build() { HFT_TRY(Model, {ffi::build_bpe(*inner_)}); }

    BpeBuilder& files(nonstd::string_view vocab, nonstd::string_view merges) {
        ffi::files_bpe(*inner_, to_rust_string(vocab), to_rust_string(merges));
        return *this;
    }

    /// Vocab must be any container of std::pair<S, uint32_t>,
    /// Merges any container of std::pair<S, S>,
    /// where to_rust_string(S) returns a rust::String
    template <typename Vocab, typename Merges>
    BpeBuilder& vocab_and_merges(Vocab vocab, Merges merges) {
        rust::Vec<ffi::KVStringU32> vocab_ffi;
        vocab_ffi.reserve(vocab.size());
        for (auto& kv : vocab) {
            vocab_ffi.push_back(
                {to_rust_string(kv.first), to_rust_string(kv.second)});
        }

        rust::Vec<ffi::StringString> merges_ffi;
        merges_ffi.reserve(merges.size);
        for (auto& merge : merges) {
            merges_ffi.push_back(
                {to_rust_string(merge.first), to_rust_string(merge.second)});
        }

        ffi::vocab_and_merges_bpe(*inner_, vocab_ffi, merges_ffi);
        return *this;
    }

    BpeBuilder& cache_capacity(size_t capacity) {
        ffi::cache_capacity_bpe(*inner_, capacity);
        return *this;
    }

    BpeBuilder& unk_token(nonstd::string_view unk_token) {
        ffi::unk_token_bpe(*inner_, to_rust_string(unk_token));
        return *this;
    }

    BpeBuilder& dropout(float dropout) {
        ffi::dropout_bpe(*inner_, dropout);
        return *this;
    }

    BpeBuilder& continuing_subword_prefix(nonstd::string_view prefix) {
        ffi::continuing_subword_prefix_bpe(*inner_, to_rust_string(prefix));
        return *this;
    }

    BpeBuilder& end_of_word_suffix(nonstd::string_view suffix) {
        ffi::end_of_word_suffix_bpe(*inner_, to_rust_string(suffix));
        return *this;
    }

    BpeBuilder& fuse_unk(bool fuse_unk) {
        ffi::fuse_unk_bpe(*inner_, fuse_unk);
        return *this;
    }
};

}  // namespace tokenizers
}  // namespace huggingface