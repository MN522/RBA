#define INTINF 9999

typedef struct EdgeRec {
	char *N;
	struct NodeRec *f,*t;
	int d;
	int com;	// �ʐM�̃N���b�N�T�C�N��
	int giveLongest;	// 1�̂Ƃ��A���̎}���Œ��o�H(LowerBound)�����肷��
	struct EdgeRec *ilist,*olist;
	struct EdgeRec *iblist,*oblist;

} EDGE;

typedef struct NodeRec {
	char *N;		// �m�[�h��
	int n;		// �z���̃C���f�b�N�X
	int LB,UB;
	int infLB,infUB;
	int Time;	// ���s�J�n����. fix=TRUE�̏ꍇ�ɈӖ�������
	int C;		// ���s����(���̉��Z��ނɑ΂��鍂�����Z��̏ꍇ)
	int L;		// �p�C�v���C���Ԋu(���̉��Z��ނɑ΂��鍂�����Z��̏ꍇ)
	int nPriority;	//���X�g�X�P�W���[�����O�p�D��x
	int *aSlotH, *aSlotL;	//�X�P�W���[���\�Ȏ����̔z��
	char *T;
	int fix;	// ���̃m�[�h�̎��s���������肵�Ă��邩(���s����0�̓��̓m�[�h�Ŏ���0�Ƒz��=2, ��������ς�=1, ���������蓖��=0)
	int pindex;	// ���̃m�[�h�����蓖�Ă�ꂽ���Z��C���f�b�N�X
	int proc;	// ���̃m�[�h�����s���鉉�Z����
	struct NodeRec *prev;	// ASAP�X�P�W���[�����̍Œ��o�H��̒��O�m�[�h
	struct NodeRec *forw;	// ALAP�X�P�W���[�����̍Œ��o�H��̒��O�m�[�h

	struct EdgeRec *ilist,*olist;	//�f�[�^�}���X�g
} NODE;

typedef struct ProcTypeRec{
char *N;
char *T;	// ���: 'A', 'M'
int C;	// ���C�e���V(���͂���o�͂܂ł̒x���N���b�N��)
int L;	// �p�C�v���C���Ԋu(���̉��Z�J�n�܂ł̍ŒZ�N���b�N���Ԋu)
int A;	// �R�X�g(�ʐ�)
} PROCTYPE;

typedef struct RegTypeRec{
char *N;
int A;	// �R�X�g(�ʐ�)
} REGTYPE;
