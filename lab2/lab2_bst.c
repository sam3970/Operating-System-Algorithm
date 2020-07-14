/*
*   Operating System Lab
*       Lab2 (Synchronization)
*       Student id : 32131706,32147356
*       Student name : 백준하,이상규
*
*   lab2_bst.c :
*       - thread-safe bst code.
*       - coarse-grained, fine-grained lock code
*
*   Implement thread-safe bst for coarse-grained version and fine-grained version.
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <string.h>

#include "lab2_sync_types.h"

pthread_mutex_t mutex;
int node_count = 0;

/*
 * TODO
 *  Implement funtction which traverse BST in in-order
 *  
 *  @param lab2_tree *tree  : bst to print in-order. 
 *  @return                 : status (success or fail)
 */
int lab2_node_print_inorder(lab2_tree *tree) {
    inorder(tree->root);
    printf("\n node Number : %d",node_count);
    return 0;
}

/*
 * TODO
 *  Implement function which creates struct lab2_tree
 *  ( refer to the ./include/lab2_sync_types.h for structure lab2_tree )
 * 
 *  @return                 : bst which you created in this function.
 */
lab2_tree *lab2_tree_create() {
    // You need to implement lab2_tree_create function.
    lab2_tree *tree = (lab2_tree *)malloc(sizeof(lab2_tree));
    pthread_mutex_init(&tree->mutex, NULL); //락 초기화
    tree->root = NULL;
    node_count = 0;
    return tree;
}

/*
 * TODO
 *  Implement function which creates struct lab2_node
 *  ( refer to the ./include/lab2_sync_types.h for structure lab2_node )
 *
 *  @param int key          : bst node's key to creates
 *  @return                 : bst node which you created in this function.
 */
lab2_node * lab2_node_create(int key) {
    // You need to implement lab2_node_create function.
    lab2_node *n = (lab2_node *)malloc(sizeof(lab2_node));
    pthread_mutex_init(&n->mutex, NULL); //락 초기화
    n->key = key;
    n->left = NULL;
    n->right = NULL;
    return n;
}

/* 
 * TODO
 *  Implement a function which insert nodes from the BST. 
 *  
 *  @param lab2_tree *tree      : bst which you need to insert new node.
 *  @param lab2_node *new_node  : bst node which you need to insert. 
 *  @return                 : satus (success or fail)
 */
/* 기본 노드 삽입*/
int lab2_node_insert(lab2_tree *tree, lab2_node *new_node){
    // You need to implement lab2_node_insert function.
    lab2_node *p = NULL,
              *t = tree->root,
              *n;
    int key = new_node->key;
    while(t != NULL){
        if(key == t->key) {
            // printf("key == t->key \n");
            return 0;
        }
        p = t;
        t = key < t->key ? t->left : t->right;
    }
    n = new_node;
    if(p == NULL) {
        tree->root = n;
        // printf("t = new_node; \n");
        node_count++;
        return 0;
    }
    if(key < p->key){
        p->left = n;
    } else {
        p->right = n;
    }
    // printf("insert key : %d\n", key);
    node_count++;
    return 0;
}

/* 
 * TODO
 *  Implement a function which insert nodes from the BST in fine-garined manner.
 *
 *  @param lab2_tree *tree      : bst which you need to insert new node in fine-grained manner.
 *  @param lab2_node *new_node  : bst node which you need to insert. 
 *  @return                     : status (success or fail)
 */
/*fine grained lock insert(모든 critical section에 락 삽입)*/
int lab2_node_insert_fg(lab2_tree *tree, lab2_node *new_node){
    // You need to implement lab2_node_insert function.
	int element = new_node->key;
	lab2_node * search_node = tree->root;
	lab2_node * parent_node = 0;

	/*
		루트가 null일 경우 root에 새 노드를 바로 넣어준다
		이 때, 여러 메소드가 이 작업을 동시에 수행하지 않도록 lock을 걸어주는데
		다른 쓰레드가 이미 해당 작업을 완료한 후에, 다른 쓰레드에서 이 작업이
		중복 수행되지 않도록, 다시 한번 조건을 체크한다.
	*/
	if (!(tree->root)) {
		pthread_mutex_lock(&(tree->mutex));
		if (!tree->root) {
			tree->root = new_node;
			node_count++;
			pthread_mutex_unlock(&(tree->mutex));
			return 0;
		}
		pthread_mutex_unlock(&(tree->mutex));
	}

	while (search_node) {
		pthread_mutex_lock(&(search_node->mutex));
		//탐색 도중에 자식 노드가 변경되지 않도록 lock을 걸어준다.

		parent_node = search_node;
		if (search_node->key > element) search_node = search_node->left;
		else if (search_node->key < element) search_node = search_node->right;
		else {
			//이진 탐색트리는 중복key값을 허용하지 않는다
			pthread_mutex_unlock(&(search_node->mutex));
			return -1;
		}
		pthread_mutex_unlock(&(parent_node->mutex));
	}
	pthread_mutex_lock(&(parent_node->mutex));

	if (parent_node->key > element) parent_node->left = new_node;
	else parent_node->right = new_node;

	pthread_mutex_unlock(&(parent_node->mutex));
	pthread_mutex_lock(&tree->mutex);
	node_count++;
	pthread_mutex_unlock(&tree->mutex);
	return 0;
}

/* 
 * TODO
 *  Implement a function which insert nodes from the BST in coarse-garined manner.
 *
 *  @param lab2_tree *tree      : bst which you need to insert new node in coarse-grained manner.
 *  @param lab2_node *new_node  : bst node which you need to insert. 
 *  @return                     : status (success or fail)
 */
/*Coarse grained lock (큰 부분에 락 걸기)*/
int lab2_node_insert_cg(lab2_tree *tree, lab2_node *new_node){
    // You need to implement lab2_node_insert_cg function.
    pthread_mutex_lock(&tree->mutex);//시작 부분에 락 삽입
    lab2_node *p = NULL,
              *t = tree->root,
              *n;
    int key = new_node->key;
    while(t != NULL){
        if(key == t->key) {
            // printf("key == t->key \n");
            pthread_mutex_unlock(&tree->mutex);
            return 0;
        }
        p = t;
        t = key < t->key ? t->left : t->right;
    }
    n = new_node;
    if(p == NULL) {
        tree->root = n;
        // printf("t = new_node; \n");
        node_count++;
        pthread_mutex_unlock(&tree->mutex);
        return 0;
    }
    if(key < p->key){
        p->left = n;
    } else {
        p->right = n;
    }
    // printf("insert key : %d\n", key);
    node_count++;
    pthread_mutex_unlock(&tree->mutex);//락 풀기
    return 0;
}

/* 
 * TODO
 *  Implement a function which remove nodes from the BST.
 *
 *  @param lab2_tree *tree  : bst tha you need to remove node from bst which contains key.
 *  @param int key          : key value that you want to delete. 
 *  @return                 : status (success or fail)
 */
int lab2_node_remove(lab2_tree *tree, int key) {
    // You need to implement lab2_node_remove function.
    lab2_node *p = NULL, *child, *succ, *succ_p, *t = tree->root;

    while(t != NULL && t->key != key){
        //printf("left : %d, right : %d\n", t->left->key, t->right->left->key);
        p = t;
        t = (key < t->key) ? t->left : t->right;
    }
    if(t == NULL) {
        return 0;
    }
    if( (t->left == NULL) && (t->right == NULL) ){
        if(p != NULL){
            if( p->left == t ){
                p->left = NULL;
            } else{
                p->right = NULL;
            }
        } else{
            tree->root = NULL;
        }
    } else if( (t->left == NULL) || (t->right == NULL) ){
        child = (t->left != NULL) ? t->left : t->right;
        if( p != NULL ){
            if( p->left == t ){
                p->left = child;
            } else{
                p->right = child;
            }
        } else{ 
            tree->root = child;
        }
    } else {
        succ_p = t;
        succ = t->right;
        while( succ->left != NULL ){
            succ_p = succ;
            succ = succ->left;
        }
        if( succ_p->left == succ ){
            succ_p->left = succ->right;
        } else{
            succ_p->right = succ->right;
        }
        t->key = succ->key;
        t = succ;
    }
    t=NULL;
	node_count--;
    //lab2_node_print_inorder(tree);
    return 0;
}

/* 
 * TODO
 *  Implement a function which remove nodes from the BST in fine-grained manner.
 *
 *  @param lab2_tree *tree  : bst tha you need to remove node in fine-grained manner from bst which contains key.
 *  @param int key          : key value that you want to delete. 
 *  @return                 : status (success or fail)
 */
/* fine grained lock remove */
int lab2_node_remove_fg(lab2_tree *tree, int key) {
    // You need to implement lab2_node_remove function.

	lab2_node * target = tree->root;
	lab2_node * target_parent = 0;
	lab2_node * left_max_parent;
	lab2_node * left_max;
	lab2_node * temp;
	int first = 0;

	int target_element = -1;

	if (target) {
		pthread_mutex_lock(&target->mutex);
	}
	else return target_element;

	while (target->key != key) {
		if (first)pthread_mutex_unlock(&target_parent->mutex);
		//처음에는 target_parent가 존재하지 않음. 
		first = 1;
		target_parent = target;

		if (target->key > key) target = target->left;
		else target = target->right;

		if (!target)break;
		pthread_mutex_lock(&target->mutex);
		/*
			현재 target_parent와 target에 lock걸린 상태
			target에 lock을 거는것은 탐색 도중에 target노드 자신과,
			자식노드와의 link가 변하지 않음을 보장하기 위함.
			target_parent에 lock을 거는 것은 target_parent와 그의 자식노드의 link가
			변경될 가능성이 있기 때문에, 접근을 제한하기 위함이다
		*/
	}

	if (!target) {
		pthread_mutex_unlock(&target_parent->mutex);
		return target_element;
	}
	target_element = target->key;


	if (target->left&&target->right) {
		left_max = target->left;
		left_max_parent = target;
		pthread_mutex_lock(&left_max->mutex);
		/*
			left_max와 left_max_parent도 위와 비슷한 이유로 lock과 unlcok을 반복하며 탐색
		*/
		if (left_max->right) {
			while (1) {
				left_max_parent = left_max;
				left_max = left_max->right;
				pthread_mutex_lock(&left_max->mutex);
				if (!left_max->right) {
					break;
				}

				pthread_mutex_unlock(&left_max_parent->mutex);
			}
			pthread_mutex_unlock(&left_max->mutex);
		}
		target->key = left_max->key;
		if (target_parent)pthread_mutex_unlock(&target_parent->mutex);
		if (left_max_parent != target)pthread_mutex_unlock(&target->mutex);
		target = left_max;
		target_parent = left_max_parent;
		/*
			자식노드 2개있을때의 삭제는, 삭제 대상노드의 왼쪽 서브트리의 최대노드의 값을
			삭제 대상노드에 치환시키는 것이므로, 실질적인 삭제는 left_max를 대상으로 이루어 져야 한다.
		*/
	}
	if (target->left) temp = target->left;
	else temp = target->right;

	if (target == tree->root) tree->root = temp;
	else {
		if (target == target_parent->left) target_parent->left = temp;
		else target_parent->right = temp;
	}
	pthread_mutex_unlock(&target->mutex);
	if (target_parent)pthread_mutex_unlock(&target_parent->mutex);
	lab2_node_delete(target);
	pthread_mutex_lock(&tree->mutex);
	node_count--;
	pthread_mutex_unlock(&tree->mutex);
	return target_element;
}


/* 
 * TODO
 *  Implement a function which remove nodes from the BST in coarse-grained manner.
 *
 *  @param lab2_tree *tree  : bst tha you need to remove node in coarse-grained manner from bst which contains key.
 *  @param int key          : key value that you want to delete. 
 *  @return                 : status (success or fail)
 */
/* coarse grained lock remove */
int lab2_node_remove_cg(lab2_tree *tree, int key) {
    // You need to implement lab2_node_remove_cg function.
    pthread_mutex_lock(&tree->mutex); //큰 부분에 락 삽입
    lab2_node *p = NULL, *child, *succ, *succ_p, *t = tree->root;
    
    while(t != NULL && t->key != key){
        //printf("left : %d, right : %d\n", t->left->key, t->right->left->key);
        p = t;
        t = (key < t->key) ? t->left : t->right;
    }
    if(t == NULL){
        pthread_mutex_unlock(&tree->mutex);
        return 0;
    }
    if( (t->left == NULL) && (t->right == NULL) ){
        if(p != NULL){
            if( p->left == t ){
                p->left = NULL;
            } else{
                p->right = NULL;
            }
        } else{ 
            tree->root = NULL;
        }
    } else if( (t->left == NULL) || (t->right == NULL) ){
        child = (t->left != NULL) ? t->left : t->right;
        if( p != NULL ){
            if( p->left == t ){
                p->left = child;
            } else{
                p->right = child;
            }
        } else{ 
            tree->root = child;
        }
    } else {
        succ_p = t;
        succ = t->right;
        while( succ->left != NULL ){
            succ_p = succ;
            succ = succ->left;
        }
        
        if( succ_p->left == succ ){
            succ_p->left = succ->right;
        } else{
            succ_p->right = succ->right;
        }
        
        t->key = succ->key;
        t = succ;
    }
    t=NULL;
	node_count--;
    pthread_mutex_unlock(&tree->mutex);//락 풀기
    return 0;
}


/*
 * TODO
 *  Implement function which delete struct lab2_tree
 *  ( refer to the ./include/lab2_sync_types.h for structure lab2_node )
 *
 *  @param lab2_tree *tree  : bst which you want to delete. 
 *  @return                 : status(success or fail)
 */
void lab2_tree_delete(lab2_tree *tree) {
    // You need to implement lab2_tree_delete function.
    int key;
    lab2_node *t = tree->root;
    if(t == NULL) return;
    while(t != NULL){
        key = t->key;
        //printf("key : %d\n",key);
        lab2_node_remove(tree, key);
        t = tree->root;
    }
}

/*
 * TODO
 *  Implement function which delete struct lab2_node
 *  ( refer to the ./include/lab2_sync_types.h for structure lab2_node )
 *
 *  @param lab2_tree *tree  : bst node which you want to remove. 
 *  @return                 : status(success or fail)
 */
void lab2_node_delete(lab2_node *node) {
    // You need to implement lab2_node_delete function.
        free(node);
}

 

/*
 * TODO
 *  Implement function which delete struct lab2_node
 *  ( refer to the ./include/lab2_sync_types.h for structure lab2_node )
 *
 *  @param lab2_tree *tree  : bst node which you want to remove. 
 *  @return                 : status(success or fail)
 */
void inorder(lab2_node *node){  
    if(node == NULL) return;
    inorder(node->left);
    //printf("%d ", node->key);
    inorder(node->right);
}
