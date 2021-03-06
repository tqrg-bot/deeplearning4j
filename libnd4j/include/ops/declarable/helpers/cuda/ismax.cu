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
// @author Yurii Shyrma, created on 21.09.2018
// @author raver119@gmail.com
//


#include <helpers/TAD.h>
#include<ops/declarable/helpers/ismax.h>
#include<loops/special_kernels.h>
#include <helpers/DebugHelper.h>
#include <cuda_exception.h>
#include <PointersManager.h>
#include <helpers/ConstantTadHelper.h>

namespace nd4j 	  {
namespace ops 	  {
namespace helpers {

template <typename T>
static void ismax_(nd4j::LaunchContext * context, const NDArray* input, NDArray* output, const std::vector<int>& dimensions) {
    void* extraParams = nullptr;
    bool scalarCheat = false;
    if (extraParams == nullptr) {
        scalarCheat = true;
    }
    auto stream = context->getCudaStream();

    auto xRank = input->rankOf();
    auto zRank = output->rankOf();
    auto xType = input->dataType();
    auto zType = output->dataType();
    input->syncToDevice();
    Nd4jLong* special = nullptr;
    PointersManager manager(context, "IsMaxHelper");
    if (dimensions.size() == 0) {
//        auto scalarShape = ShapeBuilders::createScalarShapeInfo(nd4j::DataType::INT64);
        /**
        * In case of vector-input for IsMax, it just turns into IndexReduce call + further filler call
        */
        auto indexMax = input->applyIndexReduce(indexreduce::IndexMax, dimensions);
        //NativeOpExecutioner::execIndexReduceScalar(context, indexreduce::IndexMax, nullptr, input->getShapeInfo(), input->getSpecialBuffer(), input->getSpecialShapeInfo(), extraParams, nullptr, scalarShape, special, nullptr);
        //Nd4jLong maxIdx = -119;
        //checkCudaErrors(cudaStreamSynchronize(*stream));
        //cudaMemcpyAsync(&maxIdx, special, sizeof(Nd4jLong), cudaMemcpyDeviceToHost, *stream);
        //checkCudaErrors(cudaStreamSynchronize(*stream));
        int targetIdx = 0;

        if (input->ordering() == 'c' || input->ordering() == 'f' && indexMax->e<Nd4jLong>(0) * shape::stride(input->getShapeInfo())[input->rankOf() - 1] >= input->lengthOf())
            targetIdx = indexMax->e<Nd4jLong>(0);
        else
            targetIdx = indexMax->e<Nd4jLong>(0) * shape::stride(input->getShapeInfo())[input->rankOf() - 1];

        dim3 launchDims(1, 512, 1024);
        BUILD_SINGLE_SELECTOR(zType, fillIsMaxGeneric, (launchDims, stream, output->specialBuffer(), output->lengthOf(), targetIdx), LIBND4J_TYPES);

        nd4j::DebugHelper::checkErrorCode(stream, "Legacy IsMax(...) failed");

        //delete[] scalarShape;
        delete indexMax;
    } else {
        Nd4jLong* hostYShapeInfo  = nullptr;
        Nd4jLong* hostTShapeInfo  = nullptr;
        int* dimension = nullptr;
        int dimensionLength = dimensions.size();
        std::vector<int> copy(dimensions);

        auto packZ = nd4j::ConstantTadHelper::getInstance()->tadForDimensions(input->getShapeInfo(), copy.data(), copy.size());


        auto indexMaxArr = input->applyIndexReduce(indexreduce::IndexMax, dimensions);
        //indexMaxArr->printIndexedBuffer("Index max!!!");
        // we call for IMax on specified dimension
        //NativeOpExecutioner::execIndexReduce(context, indexreduce::IndexMax, nullptr, input->getShapeInfo(), input->getSpecialBuffer(), input->getSpecialShapeInfo(), extraParams, nullptr, hostTShapeInfo, special, hostYShapeInfo, const_cast<int*>(dimensions.data()), (int)dimensions.size(), nullptr, nullptr);

        //DEBUG_KERNEL(stream, opNum);

        dim3 launchDims(256, 256, 16384);
        dimension = (int *) manager.replicatePointer(dimensions.data(), dimensions.size() * sizeof(int));

        // at this point, all IMax indexes are gathered, and we execute filler
        BUILD_SINGLE_SELECTOR(zType, fillDimensionalIsMaxGeneric, (launchDims, stream, indexMaxArr->specialBuffer(), output->specialBuffer(), output->specialShapeInfo(), packZ.specialShapeInfo(), dimension, dimensionLength, packZ.specialOffsets()), LIBND4J_TYPES);
        manager.synchronize();

        delete indexMaxArr;
    }
}


void ismax(nd4j::LaunchContext * context, const NDArray *input, NDArray *output, const std::vector<int>& dimensions) {
    BUILD_SINGLE_SELECTOR(input->dataType(), ismax_, (context, input, output, dimensions), LIBND4J_TYPES);
}

BUILD_SINGLE_TEMPLATE(template void ismax_, (nd4j::LaunchContext * context, const NDArray *input, NDArray *output, const std::vector<int>& dimensions), LIBND4J_TYPES);

}
}
}

