/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation, 
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file GaussianJunctionTreeUnordered.h
 * @date Mar 29, 2013
 * @author Frank Dellaert
 * @author Richard Roberts
 */

#include <gtsam/linear/GaussianFactorGraphUnordered.h>
#include <gtsam/linear/GaussianBayesTreeUnordered.h>
#include <gtsam/inference/JunctionTreeUnordered.h>

namespace gtsam {

  // Forward declarations
  class GaussianEliminationTreeUnordered;

  /**
   * A ClusterTree, i.e., a set of variable clusters with factors, arranged in a tree, with
   * the additional property that it represents the clique tree associated with a Bayes net.
   *
   * In GTSAM a junction tree is an intermediate data structure in multifrontal
   * variable elimination.  Each node is a cluster of factors, along with a
   * clique of variables that are eliminated all at once. In detail, every node k represents
   * a clique (maximal fully connected subset) of an associated chordal graph, such as a
   * chordal Bayes net resulting from elimination.
   *
   * The difference with the BayesTree is that a JunctionTree stores factors, whereas a
   * BayesTree stores conditionals, that are the product of eliminating the factors in the
   * corresponding JunctionTree cliques.
   *
   * The tree structure and elimination method are exactly analagous to the EliminationTree,
   * except that in the JunctionTree, at each node multiple variables are eliminated at a time.
   *
   * \addtogroup Multifrontal
   * \nosubgrouping
   */
  class GTSAM_EXPORT GaussianJunctionTreeUnordered :
    public JunctionTreeUnordered<GaussianBayesTreeUnordered, GaussianFactorGraphUnordered> {
  public:
    typedef JunctionTreeUnordered<GaussianBayesTreeUnordered, GaussianFactorGraphUnordered> Base; ///< Base class
    typedef GaussianJunctionTreeUnordered This; ///< This class
    typedef boost::shared_ptr<This> shared_ptr; ///< Shared pointer to this class
    
    /**
    * Build the elimination tree of a factor graph using pre-computed column structure.
    * @param factorGraph The factor graph for which to build the elimination tree
    * @param structure The set of factors involving each variable.  If this is not
    * precomputed, you can call the Create(const FactorGraph<DERIVEDFACTOR>&)
    * named constructor instead.
    * @return The elimination tree
    */
    GaussianJunctionTreeUnordered(const GaussianEliminationTreeUnordered& eliminationTree);

    /** Copy constructor - makes a deep copy of the tree structure, but only pointers to factors are
     *  copied, factors are not cloned. */
    GaussianJunctionTreeUnordered(const This& other);

    /** Assignment operator - makes a deep copy of the tree structure, but only pointers to factors are
     *  copied, factors are not cloned. */
    This& operator=(const This& other);
  };

}
