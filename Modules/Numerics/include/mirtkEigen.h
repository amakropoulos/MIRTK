/*
 * Medical Image Registration ToolKit (MIRTK)
 *
 * Copyright 2008-2015 Imperial College London
 * Copyright 2008-2015 Daniel Rueckert, Julia Schnabel
 * Copyright 2013-2015 Andreas Schuh
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * \file  mirtkEigen.h
 * \brief Interface to Eigen3 library.
 *
 * \attention Include this header in internal files such as .cc translation units only!
 */

#ifndef MIRTK_Eigen_H
#define MIRTK_Eigen_H

#include <mirtkMatrix.h>
#include <mirtkVector.h>

#include <Eigen/Core>


namespace mirtk {


const double EIGEN_TOL = 1.0e-5;


// -----------------------------------------------------------------------------
inline Eigen::VectorXd VectorToEigen(const Vector &a)
{
  Eigen::VectorXd b(a.Rows());
  for (int i = 0; i < a.Rows(); ++i) b(i) = a(i);
  return b;
}

// -----------------------------------------------------------------------------
inline Vector EigenToVector(const Eigen::VectorXd &a)
{
  Vector b(a.size());
  for (int i = 0; i < b.Rows(); ++i) b(i) = a(i);
  return b;
}

// -----------------------------------------------------------------------------
inline Eigen::MatrixXd MatrixToEigen(const Matrix &a)
{
  Eigen::MatrixXd b(a.Rows(), a.Cols());
  for (int i = 0; i < a.Rows(); ++i)
  for (int j = 0; j < a.Cols(); ++j) {
    b(i, j) = a(i, j);
  }
  return b;
}

// -----------------------------------------------------------------------------
inline Matrix EigenToMatrix(const Eigen::MatrixXd &a)
{
  Matrix b(a.rows(), a.cols());
  for (int i = 0; i < b.Rows(); ++i)
  for (int j = 0; j < b.Cols(); ++j) {
    b(i, j) = a(i, j);
  }
  return b;
}


} // namespace mirtk

#endif // MIRTK_Eigen_H
