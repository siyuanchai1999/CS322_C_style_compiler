#include "utils.h"

    // template<class T>
    // void set_union(std::set<T> & srcA, std::set<T> & srcB, std::set<T> & target){
    //     std::vector<T> output_vec = std::vector<T>(srcA.size() + srcB.size());
    //     typename std::vector<T>::iterator it;

    //     it = std::set_union(
    //         srcA.begin(), srcA.end(),
    //         srcB.begin(), srcB.end(),
    //         output_vec.begin()
    //     );

    //     output_vec.resize(it - output_vec.begin());

    //     target = std::set<T>(output_vec.begin(), output_vec.end());
    // }


    // template<class T>
    // void set_intersect(std::set<T> & srcA, std::set<T> & srcB, std::set<T> & target){
    //     std::vector<T> output_vec = std::vector<T>(srcA.size() + srcB.size());
    //     typename std::vector<T>::iterator it;

    //     it = std::set_intersection(
    //         srcA.begin(), srcA.end(),
    //         srcB.begin(), srcB.end(),
    //         output_vec.begin()
    //     );

    //     output_vec.resize(it - output_vec.begin());
        
    //     target = std::set<T>(output_vec.begin(), output_vec.end());
    // }

    // template<class T>
    // void set_diff(std::set<T> & srcA, std::set<T> & srcB, std::set<T> & target){
    //     std::vector<T> output_vec = std::vector<T>(srcA.size() + srcB.size());
    //     typename std::vector<T>::iterator it;

    //     it = std::set_difference(
    //         srcA.begin(), srcA.end(),
    //         srcB.begin(), srcB.end(),
    //         output_vec.begin()
    //     );

    //     output_vec.resize(it - output_vec.begin());
        
    //     target = std::set<T>(output_vec.begin(), output_vec.end());
    // }