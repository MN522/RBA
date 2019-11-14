#define INTINF 9999

typedef struct EdgeRec {
	char *N;
	struct NodeRec *f,*t;
	int d;
	int com;	// 通信のクロックサイクル
	int giveLongest;	// 1のとき、この枝が最長経路(LowerBound)を決定する
	struct EdgeRec *ilist,*olist;
	struct EdgeRec *iblist,*oblist;

} EDGE;

typedef struct NodeRec {
	char *N;		// ノード名
	int n;		// 配列上のインデックス
	int LB,UB;
	int infLB,infUB;
	int Time;	// 実行開始時刻. fix=TRUEの場合に意味を持つ
	int C;		// 実行時間(この演算種類に対する高速演算器の場合)
	int L;		// パイプライン間隔(この演算種類に対する高速演算器の場合)
	int nPriority;	//リストスケジューリング用優先度
	int *aSlotH, *aSlotL;	//スケジュール可能な時刻の配列
	char *T;
	int fix;	// このノードの実行時刻が決定しているか(実行時間0の入力ノードで時刻0と想定=2, 時刻決定済み=1, 時刻未割り当て=0)
	int pindex;	// このノードが割り当てられた演算器インデックス
	int proc;	// このノードを実行する演算器種類
	struct NodeRec *prev;	// ASAPスケジュール時の最長経路上の直前ノード
	struct NodeRec *forw;	// ALAPスケジュール時の最長経路上の直前ノード

	struct EdgeRec *ilist,*olist;	//データ枝リスト
} NODE;

typedef struct ProcTypeRec{
char *N;
char *T;	// 種類: 'A', 'M'
int C;	// レイテンシ(入力から出力までの遅延クロック数)
int L;	// パイプライン間隔(次の演算開始までの最短クロック数間隔)
int A;	// コスト(面積)
} PROCTYPE;

typedef struct RegTypeRec{
char *N;
int A;	// コスト(面積)
} REGTYPE;
