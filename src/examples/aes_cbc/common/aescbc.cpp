// MIT License
//
// Copyright (c) 2019 Oleksandr Tkachenko
// Cryptography and Privacy Engineering Group (ENCRYPTO)
// TU Darmstadt, Germany
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "aescbc.h"

#include "algorithm/algorithm_description.h"
#include "protocols/bmr/bmr_wire.h"
#include "protocols/boolean_gmw/boolean_gmw_wire.h"
#include "protocols/share_wrapper.h"
#include "statistics/analysis.h"
#include "statistics/run_time_statistics.h"
#include "utility/config.h"

static void check_correctness(encrypto::motion::ShareWrapper output) {
  // #!/usr/bin/env python3
  // import pyaes
  // ct = pyaes.AES(bytes(16)).encrypt(bytes(16))
  // print(''.join(f'{b:08b}'[::-1] for b in reversed(ct)))
  constexpr auto kCorrectionBits =
      "01110100110101000010110001010011100110100101111100110010000100011101110000110100010100011111"
      "011100101011110100101001011101100110";
  const auto values{output.As<std::vector<encrypto::motion::BitVector<>>>()};
  for (std::size_t wire_i = 0; wire_i < 128; ++wire_i) {
    for (std::size_t simd_j = 0; simd_j < output->GetNumberOfSimdValues(); ++simd_j) {
      auto computed_bit = values[wire_i].Get(simd_j);
      if ((kCorrectionBits[wire_i] == '1') != computed_bit) {
        std::cerr << fmt::format("Computation not correct at output bit {} and SIMD value {}\n",
                                 wire_i, simd_j);
        std::exit(EXIT_FAILURE);
      }
    }
  }
}

encrypto::motion::RunTimeStatistics EvaluateProtocol(encrypto::motion::PartyPointer& party,
                                                     std::size_t data_bytes,
                                                     bool check) {
  // TODO tests
  assert(data_bytes % 128 == 0);
  std::vector<encrypto::motion::BitVector<>> tmp_key(128, encrypto::motion::BitVector<>(1));
  std::vector<encrypto::motion::BitVector<>> tmp_iv(128, encrypto::motion::BitVector<>(1));
  std::vector<encrypto::motion::BitVector<>> tmp_data(data_bytes, encrypto::motion::BitVector<>(1));
  encrypto::motion::ShareWrapper key_input{party->In<encrypto::motion::MpcProtocol::kBooleanGmw>(tmp_key, 0)};
  encrypto::motion::ShareWrapper chaining_state{party->In<encrypto::motion::MpcProtocol::kBooleanGmw>(tmp_iv, 0)};
  const auto kPathToAlgorithm{std::string(encrypto::motion::kRootDir) +
                              "/circuits/advanced/aes_128.bristol"};
  const auto aes_algorithm{encrypto::motion::AlgorithmDescription::FromBristol(kPathToAlgorithm)};
  auto data_idx = 0;
  std::vector<encrypto::motion::ShareWrapper> block_results;
  while (data_idx < data_bytes) {
    auto chunk = std::span(tmp_data.begin() + data_idx, 128);
    encrypto::motion::ShareWrapper chunk_input{party->In<encrypto::motion::MpcProtocol::kBooleanGmw>(chunk, 0)};
    auto tmp = chaining_state ^ chunk_input;
    auto block_inp = encrypto::motion::ShareWrapper::Concatenate({key_input, tmp});
    auto block_res = block_inp.Evaluate(aes_algorithm);
    block_results.push_back(block_res);
    chaining_state = block_res;
    data_idx += 128;
  }
//  encrypto::motion::ShareWrapper output;
//  if (check) {
//    output = block_results[0].Out();
//  }
  party->Run();
  party->Finish();
//  if (check) {
//    check_correctness(output);
//  }
  const auto& statistics = party->GetBackend()->GetRunTimeStatistics();
  return statistics.front();
}
