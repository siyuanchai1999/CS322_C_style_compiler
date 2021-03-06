#pragma once
#include <set>
#include <algorithm> 
#include <vector> 

#define IN_MAP(map, key) (map.find(key) != map.end())
#define IN_SET(set, key) (set.find(key) != set.end())


// template<class T>
// void set_union(std::set<T> & srcA, std::set<T> & srcB, std::set<T> & target);


// template<class T>
// void set_intersect(std::set<T> & srcA, std::set<T> & srcB, std::set<T> & target);

// template<class T>
// void set_diff(std::set<T> & srcA, std::set<T> & srcB, std::set<T> & target);

    template<class T>
    void set_union(std::set<T> & srcA, std::set<T> & srcB, std::set<T> & target){
        std::vector<T> output_vec = std::vector<T>(srcA.size() + srcB.size());
        typename std::vector<T>::iterator it;

        it = std::set_union(
            srcA.begin(), srcA.end(),
            srcB.begin(), srcB.end(),
            output_vec.begin()
        );

        output_vec.resize(it - output_vec.begin());

        target = std::set<T>(output_vec.begin(), output_vec.end());
    }


    template<class T>
    void set_intersect(std::set<T> & srcA, std::set<T> & srcB, std::set<T> & target){
        std::vector<T> output_vec = std::vector<T>(srcA.size() + srcB.size());
        typename std::vector<T>::iterator it;

        it = std::set_intersection(
            srcA.begin(), srcA.end(),
            srcB.begin(), srcB.end(),
            output_vec.begin()
        );

        output_vec.resize(it - output_vec.begin());
        
        target = std::set<T>(output_vec.begin(), output_vec.end());
    }

    template<class T>
    bool has_intersect(std::set<T> & srcA, std::set<T> & srcB){
        for (T item : srcA) {
            if (IN_SET(srcB, item)) {
                return true;
            }
        }

        return false;
    }

    template<class T>
    void set_diff(std::set<T> & srcA, std::set<T> & srcB, std::set<T> & target){
        std::vector<T> output_vec = std::vector<T>(srcA.size() + srcB.size());
        typename std::vector<T>::iterator it;

        it = std::set_difference(
            srcA.begin(), srcA.end(),
            srcB.begin(), srcB.end(),
            output_vec.begin()
        );

        output_vec.resize(it - output_vec.begin());
        
        target = std::set<T>(output_vec.begin(), output_vec.end());
    }
    

    /**
     *  return the remaining contents in vec that is not in idxToRemove
     * */
    template<class T>
    std::vector<T> vector_group_remove (std::vector<T> & vec,  std::set<int32_t> idxToRemove) {
        std::vector<T> res;

        for (uint32_t i = 0; i < vec.size(); i++) {
            if (!IN_SET(idxToRemove, i)) {
                res.push_back(vec[i]);
            }
        }
    }
