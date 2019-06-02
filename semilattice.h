#ifndef SEMILATTICE_TYPESYSTEM_H
#define SEMILATTICE_TYPESYSTEM_H
#include <bits/stdc++.h>
namespace semilattice {
	using namespace std;
	template<class T>
	class Semilattice {
#define n (int)v.size()
#define VI vector<int>
		vector<T *> v;
		map<T *, int> id;
		vector<VI > ed, f;
		VI ord, q;
	public:
		Semilattice() {}
		void addv(T &a) {
			for (int i = 0; i < n; i++)
				if (v[i] == &a)
					return;
			for (int i = 0; i < n; i++)
				ed[i].push_back(0);
			v.push_back(&a);
			ed.push_back(VI(n, 0));
			id[&a] = v.size() - 1;
		}
		int topsort(VI &q) {
			VI od(n, 0);
			for (int i = 0; i < n; i++)
				for (int j = 0; j < n; j++)
					if (ed[i][j])
						od[i]++;
			q.clear();
			for (int i = 0; i < n; i++)
				if (!od[i]) {
					q.push_back(i);
				}
			if (q.size() != 1)
				return 0;
			for (int i = 0; i < (int) q.size(); i++) {
				int x = q[i];
				for (int j = 0; j < n; j++)
					if (ed[j][x]) {
						od[j]--;
						if (!od[j])
							q.push_back(j);
					}
			}
			if ((int) q.size() != n)
				return 0;
			return 1;
		}
		int get_top_ord() {
			q.assign(0, 0);
			ord.assign(n, 0);
			if (!topsort(q))
				return 0;
			for (int i = 0; i < n; i++)
				ord[q[i]] = i;
			return 1;
		}
		void addedge(T &a, T &b) { ed[id[&a]][id[&b]] = 1; }
		int check() {
			f = ed;
			if (!get_top_ord())
				return 0;
			for (int i = 0; i < n; i++)
				f[i][i] = 1;
			for (int k = 0; k < n; k++)
				for (int i = 0; i < n; i++)
					for (int j = 0; j < n; j++)
						f[i][j] |= f[i][k] && f[k][j];
			for (int i = 0; i < n; i++)
				for (int j = i + 1; j < n; j++) {
					VI cur(n, 0);
					for (int k = 0; k < n; k++)
						cur[k] = f[k][i] && f[k][j];
					int min_x = -1;
					for (int k = 0; k < n; k++)
						if (cur[k])
							if (min_x == -1 || ord[min_x] > ord[k])
								min_x = k;
					if (min_x == -1)
						continue;
					int bad = 0;
					// cout<<"==="<<i<<" "<<j<<endl;
					for (int k = 0; k < n; k++) {
						// cout<<cur[k]<<" "<<f[k][min_x]<<endl;
						if (cur[k] != f[k][min_x])
							bad = 1;
					}
					if (bad)
						return 0;
				}
			return 1;
		}
		void dfs(int x, int goal, VI &vis, VI &pre, int &ok) {
			vis[x] = 1;
			if (x == goal)
				ok = 1;
			if (ok)
				return;
			for (int i = 0; i < n; i++)
				if (ed[x][i] && !vis[i]) {
					pre[i] = x;
					dfs(i, goal, vis, pre, ok);
				}
		}
		vector<T *> get_path(int x, int goal) {
			VI vis(n, 0), pre(n, 0);
			int ok = 0;
			dfs(x, goal, vis, pre, ok);
			int tmp = goal;
			vector<T *> res(0);
			while (tmp != x) {
				res.push_back(v[tmp]);
				tmp = pre[tmp];
			}
			res.push_back(v[x]);
			reverse(res.begin(), res.end());
			// for(auto i : res)cout<<*i<<" "; cout<<endl;
			return res;
		}
		pair<vector<T *>, vector<T *>>
		least_upper_bound(T &a, T &b) { // path a->goal and path b->goal
			// require check before
			int x = id[&a], y = id[&b], goal = -1;
			for (int i = 0; i < n; i++)
				if (f[x][i] && f[y][i])
					if (goal == -1 || ord[i] > ord[goal])
						goal = i;
			// cout<<*v[goal]<<endl;
			return make_pair(get_path(x, goal), get_path(y, goal));
		}
#undef n
#undef VI
	};
} // namespace semilattice
#endif