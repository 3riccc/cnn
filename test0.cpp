#include <math.h>
#include <vector>
#include <climits>

using namespace std;

#define SIGMOID(x) (1.7159*tanh(0.66666667*x))
#define DSIGMOID(S) (0.66666667/1.7159*(1.7159+(S))*(1.7159-(S)))  // derivative of the sigmoid as a function of the sigmoid's output
#define NULL 0 

// forward declarations

class NNLayer;
class NNWeight;
class NNNeuron;
class NNConnection;


// helpful typedef's

typedef std::vector< NNLayer* >  VectorLayers;
typedef std::vector< NNWeight* >  VectorWeights;
typedef std::vector< NNNeuron* >  VectorNeurons;
typedef std::vector< NNConnection > VectorConnections;
typedef unsigned int UINT;
typedef const char * LPCTSTR;

// neural network class
class NeuralNetwork{
public:
	NeuralNetwork();
	virtual ~NeuralNetwork();
	// 向前传播算法
	void Calculate(double *inputVector,UINT iCount,double* outputVector = NULL,UINT oCount = 0);

	// 负反馈传播算法
	void BackPropagate(double *actualOutput,double *desireOutput, UINT count);

	VectorLayers m_Layers;
};

// layer 层
class NNLayer{
public:
	NNLayer(LPCTSTR str,NNLayer* pPrev = NULL);
	virtual ~NNLayer();

	// 向前传播算法
	void Calculate();
	// 负反馈传播算法
	void BackPropagate(std::vector< double >& dErr_wrt_dXn/* in */,
		std::vector< double >& dErr_wrt_dXnm1 /* out */,
		double etaLearningRate);

	NNLayer* m_pPrevLayer;
	VectorNeurons m_Neurons;
	VectorWeights m_Weights;
};

// 神经元
class NNNeuron{
public:
	NNNeuron( LPCTSTR str);
	virtual ~NNNeuron();

	void AddConnection(UINT iNeuron,UINT iWeight);
	void AddConnection(NNConnection const & conn);

	double output;

	VectorConnections m_Connections;
};

// 不同层之间的连接
class NNConnection{
public:
	NNConnection(UINT neuron = ULONG_MAX,UINT weight = ULONG_MAX);
	virtual ~NNConnection();

	UINT NeuronIndex;
	UINT WeightIndex;
};

// 权重
class NNWeight{
public:
	NNWeight(LPCTSTR str,double val= 0.0);
	virtual ~NNWeight();

	double value;
};

int main(){
	return 0;
};