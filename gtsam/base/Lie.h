/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation, 
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file Lie.h
 * @brief Base class and basic functions for Lie types
 * @author Richard Roberts
 * @author Alex Cunningham
 */


#pragma once

#include <gtsam/base/Manifold.h>
#include <gtsam/base/Group.h>

namespace gtsam {

/// A CRTP helper class that implements Lie group methods
/// Prerequisites: methods operator*, inverse, and AdjointMap, as well as a
/// ChartAtOrigin struct that will be used to define the manifold Chart
/// To use, simply derive, but also say "using LieGroup<Class,N>::inverse"
/// For derivative math, see doc/math.pdf
template <class Class, int N>
struct LieGroup {

  BOOST_STATIC_ASSERT_MSG(N != Eigen::Dynamic,
      "LieGroup not yet specialized for dynamically sized types.");

  enum { dimension = N };
  typedef OptionalJacobian<N, N> ChartJacobian;
  typedef Eigen::Matrix<double, N, N> Jacobian;
  typedef Eigen::Matrix<double, N, 1> TangentVector;

  const Class & derived() const {
    return static_cast<const Class&>(*this);
  }

  Class compose(const Class& g) const {
    return derived() * g;
  }

  Class between(const Class& g) const {
    return derived().inverse() * g;
  }

  Class compose(const Class& g, ChartJacobian H1,
      ChartJacobian H2 = boost::none) const {
    if (H1) *H1 = g.inverse().AdjointMap();
    if (H2) *H2 = Eigen::Matrix<double, N, N>::Identity();
    return derived() * g;
  }

  Class between(const Class& g, ChartJacobian H1,
      ChartJacobian H2 = boost::none) const {
    Class result = derived().inverse() * g;
    if (H1) *H1 = - result.inverse().AdjointMap();
    if (H2) *H2 = Eigen::Matrix<double, N, N>::Identity();
    return result;
  }

  Class inverse(ChartJacobian H) const {
    if (H) *H = - derived().AdjointMap();
    return derived().inverse();
  }

  Class expmap(const TangentVector& v) const {
    return compose(Class::Expmap(v));
  }

  TangentVector logmap(const Class& g) const {
    return Class::Logmap(between(g));
  }

  Class retract(const TangentVector& v) const {
    return compose(Class::ChartAtOrigin::Retract(v));
  }

  TangentVector localCoordinates(const Class& g) const {
    return Class::ChartAtOrigin::Local(between(g));
  }

  Class retract(const TangentVector& v, //
      ChartJacobian H1, ChartJacobian H2 = boost::none) const {
    Jacobian D_g_v;
    Class g = Class::ChartAtOrigin::Retract(v,D_g_v);
    Class h = compose(g,H1,H2);
    if (H2) *H2 = (*H2) * D_g_v;
    return h;
  }

  TangentVector localCoordinates(const Class& g, //
      ChartJacobian H1, ChartJacobian H2 = boost::none) const {
    Class h = between(g,H1,H2);
    Jacobian D_v_h;
    TangentVector v = Class::ChartAtOrigin::Local(h, D_v_h);
    if (H1) *H1 = D_v_h * (*H1);
    if (H2) *H2 = D_v_h * (*H2);
    return v;
  }

};

/// tag to assert a type is a Lie group
struct lie_group_tag: public manifold_tag, public group_tag {};

namespace internal {

/// A helper class that implements the traits interface for GTSAM lie groups.
/// To use this for your gtsam type, define:
/// template<> struct traits<Class> : public internal::LieGroupTraits<Class> {};
template<class Class>
struct LieGroupTraits : Testable<Class> {
  typedef lie_group_tag structure_category;

  /// @name Group
  /// @{
  typedef multiplicative_group_tag group_flavor;
  static Class Identity() { return Class::identity();}
  static Class Compose(const Class& m1, const Class& m2) { return m1 * m2;}
  static Class Between(const Class& m1, const Class& m2) { return m1.inverse() * m2;}
  static Class Inverse(const Class& m) { return m.inverse();}
  /// @}

  /// @name Manifold
  /// @{
  typedef Class ManifoldType;
  enum { dimension = Class::dimension };
  typedef Eigen::Matrix<double, dimension, 1> TangentVector;
  typedef OptionalJacobian<dimension, dimension> ChartJacobian;

  BOOST_STATIC_ASSERT_MSG(dimension != Eigen::Dynamic,
      "LieGroupTraits not yet specialized for dynamically sized types.");

  static int GetDimension(const Class&) {return dimension;}

  static TangentVector Local(const Class& origin, const Class& other) {
    return origin.localCoordinates(other);
  }

  static Class Retract(const Class& origin, const TangentVector& v) {
    return origin.retract(v);
  }

  static TangentVector Local(const Class& origin, const Class& other,
      ChartJacobian Horigin, ChartJacobian Hother = boost::none) {
    return origin.localCoordinates(other, Horigin, Hother);
  }

  static Class Retract(const Class& origin, const TangentVector& v,
      ChartJacobian Horigin, ChartJacobian Hv = boost::none) {
    return origin.retract(v, Horigin, Hv);
  }

  /// @}

  /// @name Lie Group
  /// @{

  static TangentVector Logmap(const Class& m, ChartJacobian Hm = boost::none) {
    return Class::Logmap(m, Hm);
  }

  static Class Expmap(const TangentVector& v, ChartJacobian Hv = boost::none) {
    return Class::Expmap(v, Hv);
  }

  static Class Compose(const Class& m1, const Class& m2, ChartJacobian H1,
      ChartJacobian H2 = boost::none) {
    return m1.compose(m2, H1, H2);
  }

  static Class Between(const Class& m1, const Class& m2, ChartJacobian H1,
      ChartJacobian H2 = boost::none) {
    return m1.between(m2, H1, H2);
  }

  static Class Inverse(const Class& m, ChartJacobian H) {
    return m.inverse(H);
  }

  /// @}

};

} // \ namepsace internal

/**
 * These core global functions can be specialized by new Lie types
 * for better performance.
 */

/** Compute l0 s.t. l2=l1*l0 */
template<class Class>
inline Class between_default(const Class& l1, const Class& l2) {
  return l1.inverse().compose(l2);
}

/** Log map centered at l0, s.t. exp(l0,log(l0,lp)) = lp */
template<class Class>
inline Vector logmap_default(const Class& l0, const Class& lp) {
  return Class::Logmap(l0.between(lp));
}

/** Exponential map centered at l0, s.t. exp(t,d) = t*exp(d) */
template<class Class>
inline Class expmap_default(const Class& t, const Vector& d) {
  return t.compose(Class::Expmap(d));
}

/**
 * Lie Group Concept
 */
template<typename T>
class IsLieGroup: public IsGroup<T>, public IsManifold<T> {
public:
  typedef typename traits_x<T>::structure_category structure_category_tag;
  typedef typename traits_x<T>::ManifoldType ManifoldType;
  typedef typename traits_x<T>::TangentVector TangentVector;
  typedef typename traits_x<T>::ChartJacobian ChartJacobian;

  BOOST_CONCEPT_USAGE(IsLieGroup) {
    BOOST_STATIC_ASSERT_MSG(
        (boost::is_base_of<lie_group_tag, structure_category_tag>::value),
        "This type's trait does not assert it is a Lie group (or derived)");

    // group opertations with Jacobians
    g = traits_x<T>::Compose(g, h, Hg, Hh);
    g = traits_x<T>::Between(g, h, Hg, Hh);
    g = traits_x<T>::Inverse(g, Hg);
    // log and exp map without Jacobians
    g = traits_x<T>::Expmap(v);
    v = traits_x<T>::Logmap(g);
    // log and expnential map with Jacobians
    g = traits_x<T>::Expmap(v, Hg);
    v = traits_x<T>::Logmap(g, Hg);
  }
private:
  T g, h;
  TangentVector v;
  ChartJacobian Hg, Hh;
};

/**
 *  Three term approximation of the Baker�Campbell�Hausdorff formula
 *  In non-commutative Lie groups, when composing exp(Z) = exp(X)exp(Y)
 *  it is not true that Z = X+Y. Instead, Z can be calculated using the BCH
 *  formula: Z = X + Y + [X,Y]/2 + [X-Y,[X,Y]]/12 - [Y,[X,[X,Y]]]/24
 *  http://en.wikipedia.org/wiki/Baker�Campbell�Hausdorff_formula
 */
/// AGC: bracket() only appears in Rot3 tests, should this be used elsewhere?
template<class T>
T BCH(const T& X, const T& Y) {
  static const double _2 = 1. / 2., _12 = 1. / 12., _24 = 1. / 24.;
  T X_Y = bracket(X, Y);
  return X + Y + _2 * X_Y + _12 * bracket(X - Y, X_Y) - _24 * bracket(Y,
      bracket(X, X_Y));
}

/**
 * Declaration of wedge (see Murray94book) used to convert
 * from n exponential coordinates to n*n element of the Lie algebra
 */
template <class T> Matrix wedge(const Vector& x);

/**
 * Exponential map given exponential coordinates
 * class T needs a wedge<> function and a constructor from Matrix
 * @param x exponential coordinates, vector of size n
 * @ return a T
 */
template <class T>
T expm(const Vector& x, int K=7) {
  Matrix xhat = wedge<T>(x);
  return T(expm(xhat,K));
}

} // namespace gtsam

/**
 * Macros for using the LieConcept
 *  - An instantiation for use inside unit tests
 *  - A typedef for use inside generic algorithms
 *
 * NOTE: intentionally not in the gtsam namespace to allow for classes not in
 * the gtsam namespace to be more easily enforced as testable
 */
#define GTSAM_CONCEPT_LIE_INST(T) template class gtsam::IsLieGroup<T>;
#define GTSAM_CONCEPT_LIE_TYPE(T) typedef gtsam::IsLieGroup<T> _gtsam_IsLieGroup_##T;
