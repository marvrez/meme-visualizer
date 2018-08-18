#ifndef SVM_H
#define SVM_H

#include <vector>

// the implementation is basically a simplification of libsvm
// found in https://github.com/cjlin1/libsvm

typedef struct {
    int index; // -1 = end of a vector
    float value;
} svm_node_t;

typedef struct {
	int l; // number of training data
    std::vector<float> y; // target values
    std::vector<std::vector<svm_node_t> > x; // training vecs
} svm_problem_t;

typedef enum { 
    C_SVC, 
    NU_SVC, 
    ONE_CLASS, 
    EPSILON_SVR, 
    NU_SVR 
} svm_type_t;

typedef enum { 
    LINEAR, 
    POLY, 
    RBF, 
    SIGMOID, 
    RBFWEIGH, 
    RBFWMATRIX, 
    PRECOMPUTED 
} kernel_type_t; 

struct svm_parameter_t {
	svm_type_t svm_type;
    kernel_type_t kernel_type;
	int degree;				/* for poly */
	float gamma;			/* for poly/rbf/sigmoid */
	float coef0;			/* for poly/sigmoid */
    std::vector<float> kernel_weight;	/* for rbfweight/rbfwmatrix */
	int kernel_dim;			/* for rbfweight/rbfwmatrix */
	bool normalize_kernel;
	float kernel_norm;

	/* these are for training only */
	float cache_size;		/* in MB */
	float eps;				/* stopping criteria */
	float C;				/* for C_SVC, EPSILON_SVR and NU_SVR */
	int nr_weight;			/* for C_SVC */
    std::vector<float> weight_label;		/* for C_SVC */
    std::vector<float> weight;			/* for C_SVC */
	float nu;				/* for NU_SVC, ONE_CLASS, and NU_SVR */
	float p;				/* for EPSILON_SVR */
	int shrinking;			/* use the shrinking heuristics */
	int probability;		/* do probability estimates */

    svm_parameter_t() : kernel_weight(0), weight_label(0), weight(0), kernel_dim(0), nr_weight(0) { };
    svm_parameter_t& operator= (const svm_parameter_t& param);
};

typedef struct {
	svm_parameter_t param;	// parameter
	int nr_class;			// number of classes, = 2 in regression/one class svm
	int l;					// total #SV
    std::vector<std::vector<svm_node_t> > SV;			// SVs (SV[l])
    std::vector<float> sv_coef;		// coefficients for SVs in decision functions (sv_coef[k-1][l])
    std::vector<float> rho;			// constants in decision functions (rho[k*(k-1)/2])
    std::vector<float> probA;		// pairwise probability information
    std::vector<float> probB;
    std::vector<float> eps;

	// for classification only
    std::vector<int> label;				// label of each class (label[k])
    std::vector<int> nSV;				// number of SVs for each class (nSV[k])
							// nSV[0] + nSV[1] + ... + nSV[k-1] = l
	int free_sv;			// 1 if svm_model is created by svm_load_model
							// 0 if svm_model is created by svm_train
} svm_model_t;

// Kernel evaluation:
// the static method k_function is for doing single kernel evaluation
// the constructor of Kernel prepares to calculate the l*l kernel matrix
// the member function get_Q is for getting one column from the Q Matrix
class QMatrix {
public:
	virtual float* get_q(int column, int len) const = 0;
	virtual float* get_qd() const = 0;
	virtual void swap_index(int i, int j) const = 0;
	virtual ~QMatrix() {}
};

// Kernel Cache:
// l is the number of total data items
// size is the cache size limit in bytes
class Cache {
public:
	Cache(int l, long long size);
	~Cache() { delete[] m_head; };

	// request data [0,len)
	// return some position p where [p,len) need to be filled
	// (p >= len if nothing needs to be filled)
	int get_data(const int index, std::vector<float>* data, int len);
	void swap_index(int i, int j);	// future_option
private:
	const int m_l;
	long long m_cache_mem_size;

    // Circular LRU linked list implementation
	struct head_t { 
        head_t* prev, *next;	
        int len; // data[0,len)
        std::vector<float> data;
	};

	head_t* m_head;
	head_t m_lru_head;
	void lru_delete(head_t* h);
	void lru_insert(head_t* h);
};

class Kernel: public QMatrix {
public:
	Kernel(int l, svm_node_t* const* x, const svm_parameter_t& param);
	virtual ~Kernel() { delete[] m_x; };

	static float k_function(const svm_node_t* x, const svm_node_t* y, const svm_parameter_t& param);

	virtual float* get_q(int column, int len) const = 0;
	virtual float* get_qd() const = 0;
	virtual void swap_index(int i, int j);

protected:
	float (Kernel::*kernel_function)(int i, int j) const;

private:
	const svm_node_t** m_x;
    std::vector<float> m_x_square;
    std::vector<float> m_kernel_weight;
	int m_dim;

	// svm_parameters
	const int m_kernel_type;
	const int m_degree;
	const float m_gamma; // same as 1 / 2*sigma^2
	const float m_coef0;
	float m_kernel_norm;

	static float dot(const svm_node_t* px, const svm_node_t* py);
	static float dot(const svm_node_t* px, const svm_node_t* py, const std::vector<float>& weight);
	static float matrix(const svm_node_t* px, const svm_node_t* py, const std::vector<float>& W, int dim);
	float kernel_linear(int i, int j) const;
	float kernel_poly(int i, int j) const;
	float kernel_rbf(int i, int j) const;
	float kernel_rbf_weight(int i, int j) const;
	float kernel_rbf_w(int i, int j) const;
	float kernel_sigmoid(int i, int j) const;
	float kernel_precomputed(const int i, int j) const;
};

svm_model_t* svm_train(const svm_problem_t& prob, const svm_parameter_t& param);
void svm_cross_validation(const svm_problem_t& prob, const svm_parameter_t& param, int num_fold, std::vector<float>* target);
void svm_leave_one_in(const svm_problem_t& prob, const svm_parameter_t& param, int num_fold, std::vector<float>* errors);
void svm_leave_one_out(const svm_problem_t& prob, const svm_parameter_t& param, int num_fold, std::vector<float>* errors);

int	svm_save_model(const char* model_file_name, const svm_model_t& model);
svm_model_t* svm_load_model(const char* model_file_name);

int	svm_save_model_binary(const char* model_file_name, const svm_model_t& model);
svm_model_t* svm_load_model_binary(const char* model_file_name);

int	svm_get_svm_type(const svm_model_t& model);
int svm_get_nr_class(const svm_model_t& model);
std::vector<int> svm_get_labels(const svm_model_t& model);
float svm_get_svr_probability(const svm_model_t& model);
float svm_get_dual_objective_function(const svm_model_t& svm);

void svm_predict_values(const svm_model_t& model, const svm_node_t& x, std::vector<float>* dec_values);
float svm_predict(const svm_model_t& model, const svm_node_t& x);
void svm_predict_votes(const svm_model_t& model, const svm_node_t& x, std::vector<float>* votes);
float svm_predict_probability(const svm_model_t& model, const svm_node_t& x, std::vector<float>* prob_estimates);


bool svm_check_probability_model(const svm_model_t& model);

#endif
