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
// Created by GS <sgazeos@gmail.com> on 3/21/2018.
//

#include "ResultSet.h"
#include <ops/declarable/helpers/matrix_diag_part.h>
#include <Status.h>

namespace nd4j {
namespace ops {
namespace helpers {


//////////////////////////////////////////////////////////////////////////
// Returns a batched matrix tensor with new batched diagonal values.
// for detailed explanations please take a look on web page: https://www.tensorflow.org/api_docs/python/tf/matrix_set_diag
template <typename T>
int _matrixDiagPart(const NDArray* input, NDArray* output) {

    auto listOut  = output->allTensorsAlongDimension({output->rankOf() - 1});
    auto listDiag = input->allTensorsAlongDimension({input->rankOf() - 2, input->rankOf() - 1});

    if (listOut->size() != listDiag->size()) {
        nd4j_printf("matrix_diag_part: Input matrix has wrong shape.", "");
        return ND4J_STATUS_VALIDATION;
    }
    int lastDimension = nd4j::math::nd4j_min(input->sizeAt(-2), input->sizeAt(-1));
    // TODO: tune this properlys
    int lO = listOut->size();
    PRAGMA_OMP_PARALLEL_FOR_IF(lO > Environment::getInstance()->tadThreshold())
    for(int i = 0; i < lO; ++i)
        for(int j = 0; j < lastDimension; ++j)
            listOut->at(i)->p(j, listDiag->at(i)->e<T>(j, j));
    
    delete listOut;
    delete listDiag;

    return Status::OK();
}

    int matrixDiagPart(nd4j::LaunchContext * context, const NDArray* input, NDArray* output) {
        BUILD_SINGLE_SELECTOR(input->dataType(), return _matrixDiagPart, (input, output), LIBND4J_TYPES);
    }

    BUILD_SINGLE_TEMPLATE(template int _matrixDiagPart, (const NDArray* input, NDArray* output), LIBND4J_TYPES);

}
}
}