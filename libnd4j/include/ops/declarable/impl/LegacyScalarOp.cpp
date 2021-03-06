/*******************************************************************************
 * Copyright (c) 2015-2018 Skymind, Inc.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License, Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 ******************************************************************************/

//
// Created by raver119 on 16.10.2017.
//

#include <ops/declarable/LegacyScalarOp.h>
#include <NDArrayFactory.h>
#include <Status.h>


namespace nd4j {
    namespace ops {
        LegacyScalarOp::LegacyScalarOp() : LegacyOp::LegacyOp(1) {
            // no-op
        }

        LegacyScalarOp::LegacyScalarOp(int opNum)  : LegacyOp::LegacyOp(1, opNum){
            // no-op
        }

        LegacyOp* LegacyScalarOp::clone() {
            return new LegacyScalarOp(this->_opNum, *this->_scalar);
        }

        LegacyScalarOp::LegacyScalarOp(int opNum, NDArray &scalar)  : LegacyOp::LegacyOp(1, opNum){
            _scalar = scalar.dup(scalar.ordering());
        }

        ShapeList *LegacyScalarOp::calculateOutputShape(ShapeList *inputShape, nd4j::graph::Context &block) {
            auto inShape = inputShape->at(0);

            Nd4jLong *newShape;
            COPY_SHAPE(inShape, newShape);

            return SHAPELIST(CONSTANT(newShape));
        }

        Nd4jStatus LegacyScalarOp::validateAndExecute(Context &block) {
            auto x = INPUT_VARIABLE(0);
            auto z = OUTPUT_VARIABLE(0);

            int opNum = block.opNum() < 0 ? this->_opNum : block.opNum();

            ExtraArguments extras(*block.getTArguments());
            PointersManager manager(block.launchContext(), "LegacyScalarOp");

            if (block.width() > 1) {
                auto y = INPUT_VARIABLE(1);

                NDArray::prepareSpecialUse({z}, {x, y});

                NativeOpExecutioner::execScalar(block.launchContext(), opNum, x->getBuffer(), x->getShapeInfo(), x->specialBuffer(), x->specialShapeInfo(), z->getBuffer(), z->getShapeInfo(), z->specialBuffer(), z->specialShapeInfo(), y->buffer(), y->shapeInfo(), y->specialBuffer(), y->specialShapeInfo(), extras.argumentsAsT(z->dataType()));

            } else if (block.getTArguments()->size() > 0) {
                auto y = NDArrayFactory::create(x->dataType(), T_ARG(0), block.launchContext());

                NDArray::prepareSpecialUse({z}, {x, &y});

                NativeOpExecutioner::execScalar(block.launchContext(), opNum, x->getBuffer(), x->getShapeInfo(), x->specialBuffer(), x->specialShapeInfo(), z->getBuffer(), z->getShapeInfo(), z->specialBuffer(), z->specialShapeInfo(), y.buffer(), y.shapeInfo(), y.specialBuffer(), y.specialShapeInfo(), extras.argumentsAsT(z->dataType(), 1));

                manager.synchronize();
            } else {
                NDArray::prepareSpecialUse({z}, {x, _scalar});

                NativeOpExecutioner::execScalar(block.launchContext(), opNum, x->getBuffer(), x->getShapeInfo(), x->specialBuffer(), x->specialShapeInfo(), z->getBuffer(), z->getShapeInfo(), z->specialBuffer(), z->specialShapeInfo(), _scalar->buffer(), _scalar->shapeInfo(), _scalar->specialBuffer(), _scalar->specialShapeInfo(), extras.argumentsAsT(z->dataType()));
            }

            manager.synchronize();
            STORE_RESULT(*z);

            return Status::OK();
        }
    }
}