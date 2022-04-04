#ifndef __BHV__
#define __BHV__
#define GLM_FORCE_SIMD_AVX2
#include <list>
#include <vector>
#include <glm/glm.hpp>
#include <thread>

#include "AABB.hpp"
#include "parameters.hpp"
#include "Drawables.hpp"
#include "Ray.hpp"

class BVHNode:public AABB{
public:    
    BVHNode * c1;
    BVHNode * c2;
    int depth;
    std::vector<Hittable*> objects;
    BVHNode(AABB& aabb):AABB(aabb){
        this->c1 = this->c2 = nullptr;
        this->objects.clear();
    }
    BVHNode(glm::vec3 min,glm::vec3 max):AABB(min,max){
        this->c1 = this->c2 = nullptr;
        this->objects.clear();
    }
    BVHNode(BVHNode* b1, BVHNode* b2){
        this->objects.clear();
        this->min = glm::min(b1->min,b2->min);
        this->max = glm::max(b1->max,b2->max);
        this->c1 = b1;
        this->c2 = b2;
    }
    bool isLeaf(){
        return (this->c1==nullptr);
    }
};

struct ret_findOptimal{
    std::list<BVHNode*>::iterator iter;
    float cost;
};
void thread_findOptimalBox(BVHNode* node,std::list<BVHNode*>* temp_queue,size_t begin, int end,struct ret_findOptimal& ret);
class BVHTree{
    BVHNode* root;
public:
    std::list<BVHNode*> queue;
    BVHTree(){
        this->queue.clear();
        root = nullptr;
    }
    void initQueue(std::vector<Hittable*>& hittables){
        for(auto iter = hittables.begin(); iter != hittables.end(); ++iter){
            auto hittable = *iter;
            AABB aabb = hittable->bounding_box();
            auto ptr_bvh = new BVHNode(aabb);
            ptr_bvh->objects.push_back(hittable);
            this->queue.push_back(ptr_bvh);
        }
    }
    float e_cost(BVHNode* b1, BVHNode* b2){
        glm::vec3 min, max;
        min = glm::min(b1->min,b2->min);
        max = glm::max(b1->min,b2->max);
        auto dist = max-min;
        return dist[0]*dist[1]*dist[2];
    }
    std::list<BVHNode*>::iterator findOptimalBox(BVHNode* node,std::list<BVHNode*>& q){
        std::list<BVHNode*>::iterator bestIter = q.begin();
        float minCost = std::numeric_limits<float>::max();

        for(auto iter = q.begin(); iter != q.end(); ++iter){
            auto candidate = *iter;
            float t_cost = this->e_cost(node,candidate);
            if(t_cost < minCost){
                bestIter = iter;
                minCost = t_cost;
            }
        }
        return bestIter;
    }
    void buildTree(){
        printf("# of hittables : %ld\n",this->queue.size());
        int d = 0;
        while(this->queue.size() > 1){
            std::list<BVHNode*> temp_queue;
            while(this->queue.size() > 1){
                BVHNode* node = this->queue.back();
                this->queue.pop_back();
                std::list<BVHNode*>::iterator iter;
                if(this->queue.size() < max_thread*4 || max_thread==-1){
                    iter = this->findOptimalBox(node,this->queue);
                }else{
                    int* idxes = new int[max_thread+1];
                    struct ret_findOptimal* rets = new struct ret_findOptimal[max_thread];
                    int chunk = this->queue.size()/max_thread;
                    for(int i=0; i<max_thread; i++)
                        idxes[i] = i*chunk;
                    idxes[max_thread] = this->queue.size();
                    std::vector<std::thread*> threads;
                    for(int i=0;i<max_thread;i++){
                        threads.push_back(
                            new std::thread(
                                thread_findOptimalBox,node,&this->queue,idxes[i],idxes[i+1],std::ref(rets[i])
                            )
                        );
                    }
                    for(auto thread_iter=threads.begin(); thread_iter!=threads.end(); thread_iter++){
                        (*thread_iter)->join();
                    }
                    float minCost = rets[0].cost;
                    auto bestIter = rets[0].iter;
                    for(int i=1;i<max_thread;i++){
                        if(rets[i].cost < minCost){
                            minCost = rets[i].cost;
                            bestIter = rets[i].iter;
                        }
                    }
                    iter = bestIter;
                }
                BVHNode* node_best = *iter;
                temp_queue.push_back(new BVHNode(node,node_best));
                this->queue.erase(iter);
            }
            d++;
            printf("Depth now : %d\n",d);
            this->queue.insert(this->queue.end(),
                temp_queue.begin(),temp_queue.end());
        }
        this->root = this->queue.front();
        this->root->depth = 0;
        std::list<BVHNode*> stack;
        stack.push_back(this->root);
        while(!stack.empty()){
            auto node = stack.back();
            stack.pop_back();
            if(!(node->isLeaf())){
                node->c1->depth = 
                node->c2->depth = 
                    (node->depth+1);
                stack.push_back(node->c1);
                stack.push_back(node->c2);
            }
        }
    }

    void pruneTree(){
        std::vector<BVHNode*> stack;
        auto node = this->root;
        stack.push_back(node);
        while(!stack.empty()){
            node = stack.back();
            stack.pop_back();
            if(!(node->isLeaf())){
                if(node->depth==KD_MAX_Depth){
                    this->mergeChildren(node);
                }
                else{
                    stack.push_back(node->c1);
                    stack.push_back(node->c2);
                }
            }else{

            }
        }
    }
    void mergeChildren(BVHNode* parent){
        std::vector<BVHNode*> leaves;
        std::vector<BVHNode*> stack;
        auto node = parent;
        stack.push_back(node->c1);
        stack.push_back(node->c2);
        while(!stack.empty()){
            node = stack.back();
            stack.pop_back();
            if(node->isLeaf()){
                leaves.push_back(node);
            }else{
                stack.push_back(node->c1);
                stack.push_back(node->c2);
                delete node;
            }
        }
        parent->c1 = parent->c2 = nullptr;
        for(auto nodeIter = leaves.begin(); nodeIter != leaves.end(); ++nodeIter){
            auto ptr_leaf = *nodeIter;
            parent->objects.insert(
                parent->objects.end(),
                ptr_leaf->objects.begin(),
                ptr_leaf->objects.end()
            );
            delete ptr_leaf;
        }
        auto dist = glm::abs(parent->max-parent->min);
        for(int d=0;d<3;d++){
            if(dist[d]<1e-3){
                parent->min[d] -= 5e-4;
                parent->max[d] += 5e-4;
            }
        }
    }
    void ray_traversal(const Ray& r,float t, std::vector<Hittable*>& hittables){
        hittables.clear();
        std::vector<BVHNode*> stack;
        BVHNode* node;
        stack.push_back(this->root);
        while(!(stack.empty())){
            node = stack.back();
            stack.pop_back();
            if(node->isLeaf()){
                if(node->hit(r,eps,1000.0f)){
                    hittables.insert(hittables.end(),
                        node->objects.begin(),
                        node->objects.end());
                }
            }else{
                if(node->hit(r,eps,1000.0f)){
                    stack.push_back(node->c1);
                    stack.push_back(node->c2);
                }
            }
        }
    }
    void draw(){
        auto node = this->root;
        std::list<BVHNode*> stack;
        stack.push_back(node);
        while(!stack.empty()){
            node = stack.back();
            stack.pop_back();
            if(node->isLeaf()){
                glPushMatrix();
                glLoadIdentity();
                glTranslatef(
                    (node->min[0] + node->max[0])/2.0f,
                    (node->min[1] + node->max[1])/2.0f,
                    (node->min[2] + node->max[2])/2.0f
                );
                glScalef(
                    (node->max[0] - node->min[0]),
                    (node->max[1] - node->min[1]),
                    (node->max[2] - node->min[2])
                );
                if(bWireMode){
                    if(render_kd_depth==-1 || render_kd_depth==node->depth)
                        glutWireCube(1.0);
                }
                glPopMatrix();
                for(auto iter = node->objects.begin(); iter!=node->objects.end(); iter++){
                    auto ptr_drawable = (Hittable*)(*iter);
                    if(ptr_drawable->hit_type == OBJ_SPHERE){
                        Sphere* ptr_sphere = (Sphere*)ptr_drawable;
                        ptr_sphere->draw();
                    }else if(ptr_drawable->hit_type == OBJ_TRIANGLE){
                        Triangle* ptr_tri = (Triangle*)ptr_drawable;
                        ptr_tri->draw();
                    }
                }        
            }else{
                    stack.push_back(node->c1);
                    stack.push_back(node->c2);
            }
        }
    }
};
float _e_cost(BVHNode* b1, BVHNode* b2){
    glm::vec3 min, max;
    min = glm::min(b1->min,b2->min);
    max = glm::max(b1->min,b2->max);
    auto dist = max-min;
    return dist[0]*dist[1]*dist[2];
}
void thread_findOptimalBox(BVHNode* node,std::list<BVHNode*>* temp_queue,size_t begin, int end,struct ret_findOptimal& ret){
    auto iterBegin = std::next(temp_queue->begin(),begin);
    auto iterEnd = std::next(temp_queue->begin(),end);
    std::list<BVHNode*>::iterator bestIter;
    float minCost = std::numeric_limits<float>::max();

    for(auto iter = iterBegin; iter!=iterEnd; ++iter){
        auto candidate = *iter;
        float t_cost = _e_cost(node,candidate);
        if(t_cost < minCost){
            bestIter = iter;
            minCost = t_cost;
        }
    }
    ret.iter = bestIter;
    ret.cost = minCost;
}
#endif