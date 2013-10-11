#ifndef CPPAD_CG_LOOP_MODEL_INCLUDED
#define CPPAD_CG_LOOP_MODEL_INCLUDED
/* --------------------------------------------------------------------------
 *  CppADCodeGen: C++ Algorithmic Differentiation with Source Code Generation:
 *    Copyright (C) 2013 Ciengis
 *
 *  CppADCodeGen is distributed under multiple licenses:
 *
 *   - Common Public License Version 1.0 (CPL1), and
 *   - GNU General Public License Version 2 (GPL2).
 *
 * CPL1 terms and conditions can be found in the file "epl-v10.txt", while
 * terms and conditions for the GPL2 can be found in the file "gpl2.txt".
 * ----------------------------------------------------------------------------
 * Author: Joao Leal
 */

namespace CppAD {

    /**
     * An atomic function for source code generation within loops
     * 
     * @author Joao Leal
     */
    template <class Base>
    class LoopModel {
    public:
        typedef CppAD::CG<Base> CGB;
        typedef Argument<Base> Arg;
    public:
        static const std::string ITERATION_INDEX_NAME;
        typedef std::pair<size_t, size_t> pairss;
    protected:
        static const std::set<size_t> EMPTYSET;
        static const std::vector<std::set<pairss> > EMPTYVECTORSETSS;
        static const std::vector<std::set<size_t> > EMPTYVECTORSETS;

    protected:
        const size_t loopId_;
        /**
         * The tape for a single loop iteration
         */
        ADFun<CGB> * const fun_;
        /**
         * Number of loop iterations
         */
        const size_t iterationCount_;
        /**
         * loop tape dependent variable count (number of equation patterns)
         */
        const size_t m_;
        /**
         * The dependent variables ([tape equation][iteration])
         */
        std::vector<std::vector<LoopPosition> > dependentIndexes_;
        /**
         * The indexed independent variables ([tape variable][iteration])
         */
        std::vector<std::vector<LoopPosition> > indexedIndepIndexes_;
        /**
         * The non-indexed independent variables ([tape variable])
         */
        std::vector<LoopPosition> nonIndexedIndepIndexes_;
        /**
         * The independent variables related with temporary variables of the
         * original model.
         */
        std::vector<LoopPosition> temporaryIndependents_;
        /**
         * Maps the original dependent variable indexes to their positions in 
         * the loop
         */
        std::map<size_t, LoopIndexedPosition> depOrigIndexes_;
        /**
         * iteration -> original indep index -> tape indexes
         */
        std::vector<std::map<size_t, std::set<size_t> > > iteration2orig2indexedIndepIndexes_;
        /**
         * Maps the original variable indexes to non-indexed variables
         */
        std::map<size_t, LoopPosition*> orig2nonIndexedIndepIndexes_;
        /**
         * Maps the temporary variable indexes to the tape indexes
         */
        std::map<size_t, LoopPosition*> orig2tempIndepIndexes_;
        /**
         * index patterns for the indexed independent variables
         */
        vector<IndexPattern*> indepIndexPatterns_;
        /**
         * index pattern for the dependent variables
         */
        vector<IndexPattern*> depIndexPatterns_;
        /**
         * Jacobian sparsity pattern of the tape
         */
        vector<std::set<size_t> > jacTapeSparsity_;
        bool jacSparsity_;
        /**
         * hessian sparsity pattern of the tape
         */
        vector<std::set<size_t> > hessTapeSparsity_;
        bool hessSparsity_;
        /**
         * indexed hessian elements
         * [{orig J1, orig J2}] -> [iteration -> [{tape J1, tape J2}]]
         */
        std::map<pairss, std::vector<std::set<pairss> > > hessOrig2Iter2TapeJ1TapeJ2_;
        /**
         * indexed hessian elements
         * [{orig J1, orig J2}] -> [iteration -> [{tape J1}]]
         */
        std::map<pairss, std::vector<std::set<size_t> > > hessOrig2Iter2TapeJ1OrigJ2_;
        /**
         * non-indexed hessian elements
         * [{orig J1, orig J2}] -> [iteration -> [{tape J2}]]
         */
        std::map<pairss, std::vector<std::set<size_t> > > hessOrig2Iter2OrigJ1TapeJ2_;
        /**
         * non-indexed hessian elements
         * [{orig J1, orig J2}]
         */
        std::set<pairss> hessOrigJ1OrigJ2_;
        /**
         * temporary hessian elements
         * [{k1, orig J2}] -> [iteration -> [{tape J2}]]
         */
        std::map<pairss, std::vector<std::set<size_t> > > hessOrig2Iter2TempTapeJ2_;

    public:

        /**
         * Creates a new atomic function that is responsible for defining the
         * dependencies to calls of a user atomic function.
         * 
         * @param name The atomic function name.
         * @param fun
         * @param iterationCount
         * @param dependentOrigIndexes
         * @param indexedIndepOrigIndexes
         * @param nonIndexedIndepOrigIndexes
         * @param temporaryIndependents
         */
        LoopModel(ADFun<CGB>* fun,
                  size_t iterationCount,
                  const std::vector<std::vector<size_t> >& dependentOrigIndexes,
                  const std::vector<std::vector<size_t> >& indexedIndepOrigIndexes,
                  const std::vector<size_t>& nonIndexedIndepOrigIndexes,
                  const std::vector<size_t>& temporaryIndependents) :
            loopId_(createNewLoopId()),
            fun_(fun),
            iterationCount_(iterationCount),
            m_(dependentOrigIndexes.size()),
            dependentIndexes_(m_, std::vector<LoopPosition>(iterationCount)),
            indexedIndepIndexes_(indexedIndepOrigIndexes.size(), std::vector<LoopPosition>(iterationCount)),
            nonIndexedIndepIndexes_(nonIndexedIndepOrigIndexes.size()),
            temporaryIndependents_(temporaryIndependents.size()),
            iteration2orig2indexedIndepIndexes_(iterationCount),
            jacSparsity_(false),
            hessSparsity_(false) {
            CPPADCG_ASSERT_KNOWN(fun != NULL, "fun cannot be NULL");

            /**
             * dependents
             */
            for (size_t i = 0; i < m_; i++) {
                for (size_t it = 0; it < iterationCount_; it++) {
                    size_t orig = dependentOrigIndexes[i][it];
                    dependentIndexes_[i][it] = LoopPosition(i, orig);
                    depOrigIndexes_[orig] = LoopIndexedPosition(dependentIndexes_[i][it].tape,
                                                                dependentIndexes_[i][it].original,
                                                                it);
                }
            }

            /**
             * independents
             */
            size_t nIndexed = indexedIndepOrigIndexes.size();

            // indexed
            for (size_t it = 0; it < iterationCount_; it++) {
                for (size_t j = 0; j < nIndexed; j++) {
                    size_t orig = indexedIndepOrigIndexes[j][it];
                    indexedIndepIndexes_[j][it] = LoopPosition(j, orig);
                    iteration2orig2indexedIndepIndexes_[it][orig].insert(j);
                }
            }

            // nonindexed
            size_t nNonIndexed = nonIndexedIndepOrigIndexes.size();
            for (size_t j = 0; j < nNonIndexed; j++) {
                size_t orig = nonIndexedIndepOrigIndexes[j];
                nonIndexedIndepIndexes_[j] = LoopPosition(nIndexed + j, orig);
                orig2nonIndexedIndepIndexes_[orig] = &nonIndexedIndepIndexes_[j];
            }

            // temporary
            for (size_t j = 0; j < temporaryIndependents.size(); j++) {
                size_t k = temporaryIndependents[j];
                temporaryIndependents_[j] = LoopPosition(nIndexed + nNonIndexed + j, k);
                orig2tempIndepIndexes_[k] = &temporaryIndependents_[j];
            }
        }

        /**
         * Provides a unique identifier for this loop.
         * 
         * @return a unique identifier ID
         */
        inline size_t getLoopId() const {
            return loopId_;
        }

        inline const size_t getIterationCount() const {
            return iterationCount_;
        }

        inline ADFun<CGB>& getTape() const {
            return *fun_;
        }

        inline size_t getTapeDependentCount() const {
            return m_;
        }

        inline size_t getTapeIndependentCount() const {
            return fun_->Domain();
        }

        /**
         * Provides the dependent variables indexes ([tape equation][iteration])
         */
        inline const std::vector<std::vector<LoopPosition> >& getDependentIndexes() const {
            return dependentIndexes_;
        }

        /**
         * Provides the indexed independent variables ([tape variable][iteration])
         */
        inline const std::vector<std::vector<LoopPosition> >& getIndexedIndepIndexes() const {
            return indexedIndepIndexes_;
        }

        /**
         * Provides the non-indexed independent variables ([tape variable])
         */
        inline const std::vector<LoopPosition>& getNonIndexedIndepIndexes() const {
            return nonIndexedIndepIndexes_;
        }

        /**
         * Provides the independent variables related with temporary variables of the
         * original model.
         */
        inline const std::vector<LoopPosition>& getTemporaryIndependents() const {
            return temporaryIndependents_;
        }

        /**
         * Provides the locations where a dependent variable is used
         * 
         * @param origI the dependent variable index in the original model
         * @return the locations where a dependent variable is used
         */
        inline const LoopIndexedPosition& getTapeDependentIndex(size_t origI) const {
            return depOrigIndexes_.at(origI);
        }

        inline const std::map<size_t, LoopIndexedPosition>& getOriginalDependentIndexes() const {
            return depOrigIndexes_;
        }

        /**
         * Maps the original variable indexes to non-indexed variables
         */
        inline const LoopPosition* getNonIndexedIndepIndexes(size_t origJ) const {
            std::map<size_t, LoopPosition*>::const_iterator it = orig2nonIndexedIndepIndexes_.find(origJ);
            if (it != orig2nonIndexedIndepIndexes_.end()) {
                return it->second;
            } else {
                return NULL;
            }
        }

        /**
         * Maps the temporary variable indexes to temporary variables
         */
        inline const LoopPosition* getTempIndepIndexes(size_t k) const {
            std::map<size_t, LoopPosition*>::const_iterator it = orig2tempIndepIndexes_.find(k);
            if (it != orig2tempIndepIndexes_.end()) {
                return it->second;
            } else {
                return NULL;
            }
        }

        /**
         * Finds the local tape variable indexes which use a given model
         * variable at a given iteration
         * 
         * @param origJ the index of the variable in the original model
         * @param iteration the iteration
         * @return the indexes of tape variables where the variable is used
         */
        inline const std::set<size_t>& getIndexedTapeIndexes(size_t iteration, size_t origJ) const {
            assert(iteration < iteration2orig2indexedIndepIndexes_.size());

            const std::map<size_t, std::set<size_t> >& itOrigs = iteration2orig2indexedIndepIndexes_[iteration];
            std::map<size_t, std::set<size_t> >::const_iterator it = itOrigs.find(origJ);
            if (it != itOrigs.end()) {
                return it->second;
            } else {
                return EMPTYSET;
            }
        }

        /**
         * Finds the local tape variable indexes which use a given model variable
         * 
         * @param origJ the index of the variable in the original model
         * @return all the indexed tape variables for each iteration where the
         *         variable is used
         */
        inline std::map<size_t, std::set<size_t> > getIndexedTapeIndexes(size_t origJ) const {
            std::map<size_t, std::set<size_t> > iter2TapeJs;

            for (size_t iter = 0; iter < iterationCount_; iter++) {
                const std::map<size_t, std::set<size_t> >& itOrigs = iteration2orig2indexedIndepIndexes_[iter];
                std::map<size_t, std::set<size_t> >::const_iterator it = itOrigs.find(origJ);
                if (it != itOrigs.end()) {
                    iter2TapeJs[iter] = it->second;
                }
            }

            return iter2TapeJs;
        }

        /**
         * 
         * @param origJ1
         * @param origJ2
         * @return maps each iteration to the pair of tape indexes present in 
         *         the Hessian
         */
        inline const std::vector<std::set<pairss> >& getHessianIndexedIndexedTapeIndexes(size_t origJ1,
                                                                                         size_t origJ2) const {
            pairss orig(origJ1, origJ2);

            std::map<pairss, std::vector<std::set<pairss> > >::const_iterator it;
            it = hessOrig2Iter2TapeJ1TapeJ2_.find(orig);
            if (it != hessOrig2Iter2TapeJ1TapeJ2_.end()) {
                return it->second;
            } else {
                return EMPTYVECTORSETSS;
            }
        }

        inline const std::vector<std::set<size_t> >& getHessianIndexedNonIndexedTapeIndexes(size_t origJ1,
                                                                                            size_t origJ2) const {
            pairss orig(origJ1, origJ2);

            std::map<pairss, std::vector<std::set<size_t> > > ::const_iterator it;
            it = hessOrig2Iter2TapeJ1OrigJ2_.find(orig);
            if (it != hessOrig2Iter2TapeJ1OrigJ2_.end()) {
                return it->second;
            } else {
                return EMPTYVECTORSETS;
            }
        }

        inline const std::vector<std::set<size_t> >& getHessianNonIndexedIndexedTapeIndexes(size_t origJ1,
                                                                                            size_t origJ2) const {
            pairss orig(origJ1, origJ2);

            std::map<pairss, std::vector<std::set<size_t> > >::const_iterator it;
            it = hessOrig2Iter2OrigJ1TapeJ2_.find(orig);
            if (it != hessOrig2Iter2OrigJ1TapeJ2_.end()) {
                return it->second;
            } else {
                return EMPTYVECTORSETS;
            }
        }

        inline const std::set<std::pair<size_t, size_t> >& getHessianNonIndexedNonIndexedIndexes() const {
            return hessOrigJ1OrigJ2_;
        }

        inline const std::vector<std::set<size_t> >& getHessianTempIndexedTapeIndexes(size_t k1,
                                                                                      size_t origJ2) const {
            pairss pos(k1, origJ2);

            std::map<pairss, std::vector<std::set<size_t> > >::const_iterator it;
            it = hessOrig2Iter2TempTapeJ2_.find(pos);
            if (it != hessOrig2Iter2TempTapeJ2_.end()) {
                return it->second;
            } else {
                return EMPTYVECTORSETS;
            }
        }

        inline void detectIndexPatterns() {
            if (indepIndexPatterns_.size() > 0)
                return; // already done

            indepIndexPatterns_.resize(indexedIndepIndexes_.size());
            for (size_t j = 0; j < indepIndexPatterns_.size(); j++) {
                vector<size_t> indexes(iterationCount_);
                for (size_t it = 0; it < iterationCount_; it++) {
                    indexes[it] = indexedIndepIndexes_[j][it].original;
                }
                indepIndexPatterns_[j] = IndexPattern::detect(indexes);
            }

            depIndexPatterns_.resize(dependentIndexes_.size());
            for (size_t j = 0; j < depIndexPatterns_.size(); j++) {
                vector<size_t> indexes(iterationCount_);
                for (size_t it = 0; it < iterationCount_; it++) {
                    indexes[it] = dependentIndexes_[j][it].original;
                }
                depIndexPatterns_[j] = IndexPattern::detect(indexes);
            }
        }

        inline const vector<IndexPattern*>& getDependentIndexPatterns() const {
            return depIndexPatterns_;
        }

        inline const vector<IndexPattern*>& getIndependentIndexPatterns() const {
            return indepIndexPatterns_;
        }

        inline bool isTemporary(size_t tapeJ) const {
            size_t nIndexed = indexedIndepIndexes_.size();
            size_t nNonIndexed = nonIndexedIndepIndexes_.size();

            return nIndexed + nNonIndexed <= tapeJ;
        }

        inline bool isIndexedIndependent(size_t tapeJ) const {
            return tapeJ < indexedIndepIndexes_.size();
        }

        inline void evalJacobianSparsity() {
            if (!jacSparsity_) {
                jacTapeSparsity_ = extra::jacobianSparsitySet<vector<std::set<size_t> >, CGB>(*fun_);
                jacSparsity_ = true;
            }
        }

        inline const vector<std::set<size_t> >& getJacobianSparsity() const {
            return jacTapeSparsity_;
        }

        inline void evalHessianSparsity() {
            if (!hessSparsity_) {
                hessTapeSparsity_ = extra::hessianSparsitySet<vector<std::set<size_t> >, CGB>(*fun_);

                /**
                 * make a database of the hessian elements
                 */
                size_t nIndexed = indexedIndepIndexes_.size();
                size_t nNonIndexed = nonIndexedIndepIndexes_.size();
                size_t nTemp = temporaryIndependents_.size();

                for (size_t iter = 0; iter < iterationCount_; iter++) {
                    /**
                     * indexed tapeJ1
                     */
                    for (size_t tapeJ1 = 0; tapeJ1 < nIndexed; tapeJ1++) {
                        const std::set<size_t>& hessRow = hessTapeSparsity_[tapeJ1];
                        size_t j1 = indexedIndepIndexes_[tapeJ1][iter].original;

                        std::set<size_t> ::const_iterator itTape2;
                        for (itTape2 = hessRow.begin(); itTape2 != hessRow.end() && *itTape2 < nIndexed; ++itTape2) {
                            size_t j2 = indexedIndepIndexes_[*itTape2][iter].original;
                            pairss orig(j1, j2);
                            pairss tapeTape(tapeJ1, *itTape2);
                            std::vector<std::set<pairss> >& iterations = hessOrig2Iter2TapeJ1TapeJ2_[orig];
                            iterations.resize(iterationCount_);
                            iterations[iter].insert(tapeTape);
                        }

                        for (; itTape2 != hessRow.end() && *itTape2 < nIndexed + nNonIndexed; ++itTape2) {
                            size_t j2 = nonIndexedIndepIndexes_[*itTape2 - nIndexed].original;
                            pairss orig(j1, j2);
                            std::vector<std::set<size_t> >& iterations = hessOrig2Iter2TapeJ1OrigJ2_[orig];
                            iterations.resize(iterationCount_);
                            iterations[iter].insert(tapeJ1);
                        }
                    }

                    /**
                     * non-indexed tapeJ1
                     */
                    for (size_t tapeJ1 = nIndexed; tapeJ1 < nIndexed + nNonIndexed; tapeJ1++) {
                        const std::set<size_t>& hessRow = hessTapeSparsity_[tapeJ1];
                        size_t j1 = nonIndexedIndepIndexes_[tapeJ1 - nIndexed].original;

                        std::set<size_t> ::const_iterator itTape2;
                        for (itTape2 = hessRow.begin(); itTape2 != hessRow.end() && *itTape2 < nIndexed; ++itTape2) {
                            size_t j2 = indexedIndepIndexes_[*itTape2][iter].original;
                            pairss orig(j1, j2);
                            std::vector<std::set<size_t> >& iterations = hessOrig2Iter2OrigJ1TapeJ2_[orig];
                            iterations.resize(iterationCount_);
                            iterations[iter].insert(*itTape2);
                        }

                        for (; itTape2 != hessRow.end() && *itTape2 < nIndexed + nNonIndexed; ++itTape2) {
                            size_t j2 = nonIndexedIndepIndexes_[*itTape2 - nIndexed].original;
                            pairss orig(j1, j2);
                            hessOrigJ1OrigJ2_.insert(orig);
                        }
                    }

                    /**
                     * temporaries tapeJ1
                     */
                    for (size_t tapeJ1 = nIndexed + nNonIndexed; tapeJ1 < nIndexed + nNonIndexed + nTemp; tapeJ1++) {
                        const std::set<size_t>& hessRow = hessTapeSparsity_[tapeJ1];
                        size_t k1 = temporaryIndependents_[tapeJ1 - nIndexed - nNonIndexed].original;

                        std::set<size_t> ::const_iterator itTape2;
                        for (itTape2 = hessRow.begin(); itTape2 != hessRow.end() && *itTape2 < nIndexed; ++itTape2) {
                            size_t j2 = indexedIndepIndexes_[*itTape2][iter].original;
                            pairss pos(k1, j2);
                            std::vector<std::set<size_t> >& iterations = hessOrig2Iter2TempTapeJ2_[pos];
                            iterations.resize(iterationCount_);
                            iterations[iter].insert(*itTape2);
                        }

                    }
                }

                hessSparsity_ = true;
            }
        }

        inline const vector<std::set<size_t> >& getHessianSparsity() const {
            return hessTapeSparsity_;
        }

        virtual ~LoopModel() {
            delete fun_;
            for (size_t i = 0; i < indepIndexPatterns_.size(); i++) {
                delete indepIndexPatterns_[i];
            }
            for (size_t i = 0; i < depIndexPatterns_.size(); i++) {
                delete depIndexPatterns_[i];
            }
        }

        static inline void printOriginalVariableIndexes(std::ostringstream& ss,
                                                        const std::vector<LoopPosition>& indexes) {
            for (size_t iter = 0; iter < indexes.size(); iter++) {
                if (iter > 0) ss << ", ";
                ss << indexes[iter].original;
            }
        }

    private:

        static size_t createNewLoopId() {
            CPPAD_ASSERT_FIRST_CALL_NOT_PARALLEL;
            static size_t count = 0;
            count++;
            return count;
        }

        LoopModel(const LoopModel<Base>&); // not implemented

        LoopModel& operator=(const LoopModel<Base>&); // not implemented

    };

    template<class Base>
    const std::string LoopModel<Base>::ITERATION_INDEX_NAME("j");

    template<class Base>
    const std::set<size_t> LoopModel<Base>::EMPTYSET;

    template<class Base>
    const std::vector<std::set<std::pair<size_t, size_t> > > LoopModel<Base>::EMPTYVECTORSETSS;

    template<class Base>
    const std::vector<std::set<size_t> > LoopModel<Base>::EMPTYVECTORSETS;

}

#endif