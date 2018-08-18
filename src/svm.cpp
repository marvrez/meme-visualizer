#include "svm.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

#define M_TAU 1e-12
#define M_INF HUGE_VAL

template <class S, class T> inline void clone(T*& dst, S* src, int n)
{
	dst = new T[n];
	memcpy((void *)dst,(void *)src,sizeof(T)*n);
}

Cache::Cache(int l, long long size) : m_l(l), m_cache_mem_size(size)
{
	m_head = new head_t[l]();
	m_cache_mem_size /= sizeof(float);
	m_cache_mem_size -= l * sizeof(head_t) / sizeof(float);
	m_cache_mem_size = std::max(size, 2 * (long long) l);	// cache must be large enough for two columns
	m_lru_head.next = m_lru_head.prev = &m_lru_head;
}

void Cache::lru_delete(head_t* h)
{
    // delete current position
	h->prev->next = h->next;
	h->next->prev = h->prev;
}

void Cache::lru_insert(head_t* h)
{
	// insert at tail-1
	h->next = &m_lru_head;
	h->prev = m_lru_head.prev;
	h->prev->next = h->next->prev = h;
}

int Cache::get_data(const int index, std::vector<float>* data, int len)
{
	head_t* h = &m_head[index];
	if(h->len) lru_delete(h);
	int more = len - h->len;

	if(more) {
		// free old space
		while(m_cache_mem_size < more) {
			head_t* old = m_lru_head.next;
			lru_delete(old);
			m_cache_mem_size += old->len;
			old->data = std::vector<float>(0);
            old->len = 0;
		}

		h->data.resize(len);
		m_cache_mem_size -= more;
        std::swap(h->len, len);
	}

	lru_insert(h);
	*data = h->data;
	return len;
}

void Cache::swap_index(int i, int j)
{
	if(i == j) return;

	if(m_head[i].len) lru_delete(&m_head[i]);
	if(m_head[j].len) lru_delete(&m_head[j]);

    std::swap(m_head[i].data, m_head[j].data);
    std::swap(m_head[i].len, m_head[j].len);

    //TODO: check if data goes out of scope before this
	if(m_head[i].len) lru_insert(&m_head[i]);
	if(m_head[j].len) lru_insert(&m_head[j]);

	if(i > j) std::swap(i,j);
	for(head_t* h = m_lru_head.next; h != &m_lru_head; h = h->next) {
		if(h->len > i) {
			if(h->len > j) std::swap(h->data[i], h->data[j]);
			else {
				// give up
				lru_delete(h);
				m_cache_mem_size += h->len;
				h->data = std::vector<float>(0);
                h->len = 0;
			}
		}
	}
}

Kernel::Kernel(int l, svm_node_t* const* x, const svm_parameter_t& param)
    : m_kernel_type(param.kernel_type), m_degree(param.degree),
        m_gamma(param.gamma), m_coef0(param.coef0), m_kernel_weight(param.kernel_weight), m_kernel_norm(param.kernel_norm)
{
	switch(m_kernel_type) {
		case LINEAR:
			kernel_function = &Kernel::kernel_linear;
			break;
		case POLY:
			kernel_function = &Kernel::kernel_poly;
			break;
		case RBF:
			kernel_function = &Kernel::kernel_rbf;
			break;
		case RBFWEIGH:
			kernel_function = &Kernel::kernel_rbf_weight;
			break;
		case RBFWMATRIX:
			kernel_function = &Kernel::kernel_rbf_w;
			break;
		case SIGMOID:
			kernel_function = &Kernel::kernel_sigmoid;
			break;
		case PRECOMPUTED:
			kernel_function = &Kernel::kernel_precomputed;
            break;
	}

	clone(m_x,x,l);

	m_dim = param.kernel_dim;
	if(!m_dim) {
		while(m_x[0][m_dim].index != -1) m_dim++;
	}

	if(m_kernel_type == RBF) {
		m_x_square = std::vector<float>(l, 0.f);
		for(int i=0;i<l;++i) m_x_square[i] = dot(m_x[i], m_x[i]);
	}
	else if(m_kernel_type == RBFWEIGH) {
		m_x_square = std::vector<float>(l, 0.f);
		for(int i=0;i<l;++i) m_x_square[i] = dot(m_x[i], m_x[i], m_kernel_weight);
	}
	else m_x_square = std::vector<float>(0);
}

void Kernel::swap_index(int i, int j)
{
    std::swap(m_x[i], m_x[j]);
	if(m_x_square.size()) std::swap(m_x_square[i], m_x_square[j]);
}

float Kernel::kernel_linear(int i, int j) const
{
	return m_kernel_norm*dot(m_x[i],m_x[j]);
}

float Kernel::kernel_poly(int i, int j) const
{
    return m_kernel_norm*pow(m_gamma*dot(m_x[i],m_x[j])+m_coef0, m_degree);
}

float Kernel::kernel_rbf(int i, int j) const
{
    return m_kernel_norm*exp(-m_gamma*(m_x_square[i]+m_x_square[j]-2*dot(m_x[i],m_x[j])));
}

float Kernel::kernel_rbf_weight(int i, int j) const
{
    float sum = 0;
    const svm_node_t* px = m_x[i], *py = m_x[j];
    while(px->index != -1 && py->index != -1) {
        if(px->index == py->index) {
            sum += (px->value - py->value) * (px->value - py->value) * m_kernel_weight[px->index-1];
            ++px, ++py;
        }
        else {
            if(px->index > py->index) ++py;
            else ++px;
        }
    }
    return m_kernel_norm*exp(-m_gamma*sum);
}

float Kernel::kernel_rbf_w(int i, int j) const
{
	return m_kernel_norm*exp(-m_gamma*matrix(m_x[i], m_x[j], m_kernel_weight, m_dim));
}

float Kernel::kernel_sigmoid(int i, int j) const
{
	return tanh(m_gamma*dot(m_x[i],m_x[j])+m_coef0);
}

float Kernel::kernel_precomputed(int i, int j) const
{
	return m_x[i][(int)(m_x[j][0].value)].value;
}

float Kernel::dot(const svm_node_t* px, const svm_node_t* py)
{
	float sum = 0;
	while(px->index != -1 && py->index != -1) {
		if(px->index == py->index) {
			sum += px->value * py->value;
			++px, ++py;
		}
		else {
			if(px->index > py->index) ++py;
			else ++px;
		}
	}
	return sum;
}

float Kernel::dot(const svm_node_t* px, const svm_node_t* py, const std::vector<float>& weight)
{
	float sum = 0.f;
	while(px->index != -1 && py->index != -1) {
		if(px->index == py->index) {
			sum += px->value * py->value * weight[px->index];
			++px, ++py;
		}
		else {
			if(px->index > py->index) ++py;
			else ++px;
		}
	}
	return sum;
}

float Kernel::matrix(const svm_node_t* px, const svm_node_t* py, const std::vector<float>& W, int dim) {
	float sum = 0.f;
    std::vector<float> x_w(dim, 0.f);
	for (int i=0; i<dim; ++i) {
		for (int j=0; j<dim; ++j) {
			x_w[i] += (px[j].value - py[j].value)*W[j*dim + i];
		}
	}
	for (int i=0; i<dim; ++i) {
		sum += (px[i].value - py[i].value)*x_w[i];
	}
	return sum;
}


float Kernel::k_function(const svm_node_t* x, const svm_node_t* y, const svm_parameter_t& param)
{
	switch(param.kernel_type) {
		case LINEAR:
			return dot(x,y);
		case POLY:
			return pow(param.gamma*dot(x,y)+param.coef0, param.degree);
		case RBF: {
			float sum = 0.f;
			while(x->index != -1 && y->index !=-1) {
				if(x->index == y->index) {
					float d = x->value - y->value;
					sum += d*d;
					++x, ++y;
				}
				else {
					if(x->index > y->index) {
						sum += y->value * y->value;
						++y;
					}
					else {
						sum += x->value * x->value;
						++x;
					}
				}
			}

			while(x->index != -1) {
				sum += x->value * x->value;
				++x;
			}

			while(y->index != -1) {
				sum += y->value * y->value;
				++y;
			}
			if(param.normalize_kernel) return param.kernel_norm*exp(-param.gamma*sum);
			return exp(-param.gamma*sum);
		}
		case RBFWEIGH: {
            float sum = 0.f;
            while(x->index != -1 && y->index !=-1) {
                if(x->index == y->index) {
                    float d = x->value - y->value;
                    sum += d*d*param.kernel_weight[x->index-1];
                    ++x, ++y;
                }
                else {
                    if(x->index > y->index) {
                        sum += y->value * y->value * param.kernel_weight[y->index-1];
                        ++y;
                    }
                    else {
                        sum += x->value * x->value * param.kernel_weight[x->index-1];
                        ++x;
                    }
                }
            }

            while(x->index != -1) {
                sum += x->value * x->value * param.kernel_weight[x->index-1];
                ++x;
            }

            while(y->index != -1) {
                sum += y->value * y->value * param.kernel_weight[y->index-1];
                ++y;
            }
            if(param.normalize_kernel) return param.kernel_norm*exp(-param.gamma*sum);
            return exp(-param.gamma*sum);
        }
		case RBFWMATRIX: {
            int l = param.kernel_dim;
            if(!l) {
                while(x[l].index != -1) l++;
            }
            float sum = matrix(x, y, param.kernel_weight, l);
            if(param.normalize_kernel) return param.kernel_norm*exp(-param.gamma*sum);
            return exp(-param.gamma*sum);
        }
		case SIGMOID:
			return tanh(param.gamma*dot(x,y)+param.coef0);
		case PRECOMPUTED:  //x: test (validation), y: SV
			return x[(int)(y->value)].value;
		default:
			return 0;  // Unreachable
	}
}

