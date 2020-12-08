#pragma once

#include "tokenizers-cpp/common.h"
#include "tokenizers-cpp/decoders.rs.h"

namespace huggingface {
namespace tokenizers {
struct Decoder {
    HFT_FFI_WRAPPER(Decoder);

public:
    static Decoder byte_level(bool add_prefix_space, bool trim_offsets) {
        return {ffi::byte_level_decoder(add_prefix_space, trim_offsets)};
    }

    HFT_RESULT(rust::String) decode(rust::Vec<rust::String>&& tokens) {
        HFT_TRY(rust::String, ffi::decode_decoder(*inner_, std::move(tokens)));
    }
};

}  // namespace tokenizers
}  // namespace huggingface