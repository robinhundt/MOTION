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

#include "boolean_gmw_wire.h"

namespace MOTION::Wires {

GMWWire::GMWWire(ENCRYPTO::BitVector<> &&values, std::weak_ptr<Backend> backend, bool is_constant) {
  values_ = std::move(values);
  backend_ = backend;
  is_constant_ = is_constant;
  n_simd_ = values_.GetSize();
  InitializationHelper();
}

GMWWire::GMWWire(const ENCRYPTO::BitVector<> &values, std::weak_ptr<Backend> backend, bool is_constant) {
  values_ = values;
  backend_ = backend;
  is_constant_ = is_constant;
  n_simd_ = values_.GetSize();
  InitializationHelper();
}

GMWWire::GMWWire(bool value, std::weak_ptr<Backend> backend, bool is_constant) {
  values_.Append(value);
  backend_ = backend;
  is_constant_ = is_constant;
  n_simd_ = 1;
  InitializationHelper();
}

}