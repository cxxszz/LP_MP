#ifndef LP_MP_COSEGMENTATION_HXX
#define LP_MP_COSEGMENTATION_HXX

#include "problem_decomposition.hxx"

#include "factors_messages.hxx"
#include "LP_MP.h"
#include "factors/multiplex_factor.hxx"
#include "factors/simplex_factor_simd.hxx"
#include "const_array_types.h"
#include "messages/multiplex_marg_message.hxx"
#include "messages/equality_message.hxx"
#include "marginal_summation_message.hxx"
#include "problem_construction_helper.hxx"

#include "message_replicator.hxx"
#include "message_replicator_factor.hxx"

#include "assignment_problem_construction.hxx"
#include "potts_problem_construction.hxx"

namespace LP_MP {

// file format:
// first entries: assignment problem:
// problem 0
// a l r cost -> assignment from left node number l to right node number r with specified cost
//
// then: MRF-problem
// problem 1
// potts 2 i1 i2 cost
// potts factor with 2 labels between node i1 and i2
// question: Make 1 problem for left and right image separately?
//
// joining MRF and Assignment problem:
// problem 2
// {l|r}+no i
// {left|right} variable no and mrf variable i.

class Cosegmentation {
public:
   //specification for underlying factor/message-network
   // do zrobienia: hardcode 2 labels for more efficient message passing
   typedef UnaryLoop UnaryLoopType;
   typedef PairwiseLoop<0> LeftLoopType;
   typedef PairwiseLoop<1> RightLoopType;

   typedef MultiplexMargMessage<UnaryLoopType,LeftLoopType,true,false> LeftMargMessage;
   typedef MultiplexMargMessage<UnaryLoopType,RightLoopType,true,false> RightMargMessage;

   struct FMC; // forward declaration

   // factors
   typedef MultiplexFactor<std::vector<REAL>, const_ones_array, const_one> Simplex;
   typedef FactorContainer<Simplex, ExplicitRepamStorage, FMC, 0 > UnaryFactor; // 2 entries: foreground, background. do zrobienia: specialize factor
   typedef FactorContainer<Simplex, ExplicitRepamStorage, FMC, 1 > PairwiseFactor; // 4 entries for pairwise potential. do zrobienia: specialize factor
   typedef FactorContainer<Simplex, ExplicitRepamStorage, FMC, 2 > AssignmentFactor; // for each pixel: number of possible assignments

   // messages
   typedef MessageContainer<EqualityMessage, FixedMessageStorage<1>, FMC, 0 > AssignmentConstraintMessage;
   typedef MessageContainer<LeftMargMessage, MessageStorageSIMD, FMC, 1 > UnaryPairwiseMessageLeft; // unary has left_dim entries
   typedef MessageContainer<RightMargMessage, MessageStorageSIMD, FMC, 2 > UnaryPairwiseMessageRight; // unary has right_dim entries
   typedef MessageContainer<MarginalSummationMessage, FixedMessageStorage<2>, FMC, 3> MarginalSummationMessageContainer; // for bin size 1

   struct FMC {
      using factor_list = meta::list< UnaryFactor, PairwiseFactor, AssignmentFactor>;
      using msg_list = meta::list< 
                                   meta::list< AssignmentConstraintMessage, meta::size_t<2>, meta::size_t<2>, std::vector<AssignmentConstraintMessage*>, std::vector<AssignmentConstraintMessage*> >,
                                   meta::list< UnaryPairwiseMessageLeft,  meta::size_t<0>, meta::size_t<1>, std::vector<UnaryPairwiseMessageLeft*>, FixedSizeContainer<UnaryPairwiseMessageLeft*,1> >,
                                   meta::list< UnaryPairwiseMessageRight, meta::size_t<0>, meta::size_t<1>, std::vector<UnaryPairwiseMessageRight*>, FixedSizeContainer<UnaryPairwiseMessageRight*,1> >,
                                   meta::list< MarginalSummationMessageContainer, meta::size_t<0>, meta::size_t<2>, FixedSizeContainer<MarginalSummationMessageContainer*,1>, FixedSizeContainer<MarginalSummationMessageContainer*,1> >
                                      >;
      
      using lap = AssignmentProblemConstructor<FMC,2,0>;
      using potts = PottsProblemConstructor<FMC,0,1,1,2>;
      //using marg_consistency = MarginalSummationConstructor<FMC, 3, 3, 4, 0, 1>;
      using problem_decomposition = meta::list<lap, potts>;
   };
};

} // end namespace LP_MP

#endif // LP_MP_COSEGMENTATION_HXX
