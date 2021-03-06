/*
 * Medical Image Registration ToolKit (MIRTK)
 *
 * Copyright 2013-2015 Imperial College London
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

#include <mirtkSmoothnessConstraint.h>

#include <mirtkString.h>
#include <mirtkMemory.h>
#include <mirtkFreeFormTransformation.h>
#include <mirtkMultiLevelTransformation.h>
#include <mirtkObjectFactory.h>


namespace mirtk {


// Register energy term with object factory during static initialization
mirtkAutoRegisterEnergyTermMacro(SmoothnessConstraint);


// -----------------------------------------------------------------------------
SmoothnessConstraint::SmoothnessConstraint(const char *name, double weight)
:
  TransformationConstraint(name, weight),
  _WithRespectToWorld(false),
  _AnnealingRate(.0), _AnnealingWeight(1.0)
{
  _ParameterPrefix.push_back("Smoothness ");
  _ParameterPrefix.push_back("Bending energy ");
  _ParameterPrefix.push_back("Bending ");
}

// -----------------------------------------------------------------------------
bool SmoothnessConstraint::SetWithoutPrefix(const char *param, const char *value)
{
  if (strstr(param, "W.r.t world"          ) == param ||
      strstr(param, "W.r.t. world"         ) == param ||
      strstr(param, "Wrt world"            ) == param ||
      strstr(param, "With respect to world") == param) {
    return FromString(value, _WithRespectToWorld);
  }
  if (strstr(param, "Annealing rate") == param) {
    return FromString(value, _AnnealingRate);
  }
  return TransformationConstraint::SetWithoutPrefix(param, value);
}

// -----------------------------------------------------------------------------
ParameterList SmoothnessConstraint::Parameter() const
{
  ParameterList params = TransformationConstraint::Parameter();
  InsertWithPrefix(params, "W.r.t. world (experimental)",   _WithRespectToWorld);
  InsertWithPrefix(params, "Annealing rate (experimental)", _AnnealingRate);
  return params;
}

// -----------------------------------------------------------------------------
void SmoothnessConstraint::Initialize()
{
  TransformationConstraint::Initialize();
  _AnnealingWeight = 1.0;
}

// -----------------------------------------------------------------------------
bool SmoothnessConstraint::Upgrade()
{
  TransformationConstraint::Upgrade();
  if (_AnnealingRate != .0) _AnnealingWeight *= _AnnealingRate;
  return false; // whether to continue depends on similarity term, not penalty
}

// -----------------------------------------------------------------------------
double SmoothnessConstraint::Evaluate()
{
  const MultiLevelTransformation *mffd = NULL;
  const FreeFormTransformation   *ffd  = NULL;

  (mffd = MFFD()) || (ffd = FFD());

  double bending = .0;
  if (mffd) {
    for (int l = 0; l < mffd->NumberOfLevels(); ++l) {
      if (!mffd->LocalTransformationIsActive(l)) continue;
      ffd = mffd->GetLocalTransformation(l);
      bending += ffd->BendingEnergy(_ConstrainPassiveDoFs, _WithRespectToWorld);
    }
  } else if (ffd) {
    bending = ffd->BendingEnergy(_ConstrainPassiveDoFs, _WithRespectToWorld);
  }
  return _AnnealingWeight * bending;
}

// -----------------------------------------------------------------------------
void SmoothnessConstraint::EvaluateGradient(double *gradient, double, double weight)
{
  weight *= _AnnealingWeight;
  const MultiLevelTransformation *mffd = NULL;
  const FreeFormTransformation   *ffd  = NULL;

  (mffd = MFFD()) || (ffd = FFD());

  if (mffd) {
    for (int l = 0; l < mffd->NumberOfLevels(); ++l) {
      if (!mffd->LocalTransformationIsActive(l)) continue;
      ffd = mffd->GetLocalTransformation(l);
      ffd->BendingEnergyGradient(gradient, weight, _ConstrainPassiveDoFs, _WithRespectToWorld);
      gradient += ffd->NumberOfDOFs();
    }
  } else if (ffd) {
    ffd->BendingEnergyGradient(gradient, weight, _ConstrainPassiveDoFs, _WithRespectToWorld);
  }
}

// =============================================================================
// Debugging
// =============================================================================

// -----------------------------------------------------------------------------
void SmoothnessConstraint
::WriteFFDGradient(const char *fname, const FreeFormTransformation *ffd, const double *g) const
{
  typedef FreeFormTransformation::CPValue CPValue;
  typedef GenericImage<CPValue>           CPImage;
  CPValue *data = reinterpret_cast<CPValue *>(const_cast<double *>(g));
  CPImage gradient(ffd->Attributes(), data);
  gradient.Write(fname);
}

// -----------------------------------------------------------------------------
void SmoothnessConstraint::WriteGradient(const char *p, const char *suffix) const
{
  const int   sz = 1024;
  char        fname[sz];
  string _prefix = Prefix(p);
  const char  *prefix = _prefix.c_str();

  const MultiLevelTransformation *mffd = NULL;
  const FreeFormTransformation   *ffd  = NULL;

  (mffd = MFFD()) || (ffd = FFD());

  if (mffd) {
    for (int l = 0; l < mffd->NumberOfLevels(); ++l) {
      if (!mffd->LocalTransformationIsActive(l)) continue;
      ffd = mffd->GetLocalTransformation(l);
      double *gradient = CAllocate<double>(ffd->NumberOfDOFs());
      ffd->BendingEnergyGradient(gradient, 1.0, _ConstrainPassiveDoFs, _WithRespectToWorld);
      if (mffd->NumberOfActiveLevels() == 1) {
        snprintf(fname, sz, "%sgradient%s", prefix, suffix);
      } else {
        snprintf(fname, sz, "%sgradient_of_ffd_at_level_%d%s", prefix, l+1, suffix);
      }
      WriteFFDGradient(fname, ffd, gradient);
      Deallocate(gradient);
    }
  } else if (ffd) {
    snprintf(fname, sz, "%sgradient%s", prefix, suffix);
    double *gradient = CAllocate<double>(ffd->NumberOfDOFs());
    ffd->BendingEnergyGradient(gradient, 1.0, _ConstrainPassiveDoFs, _WithRespectToWorld);
    WriteFFDGradient(fname, ffd, gradient);
    Deallocate(gradient);
  }
}


} // namespace mirtk
