function isSuperset(set, subset) {
	for (let elem of subset) {
		if (!set.has(elem)) {
			return false;
		}
	}
	return true;
}
function union(setA, setB) {
	let _union = new Set(setA);
	for (let elem of setB) {
		_union.add(elem);
	}
	return _union;
}
function intersection(setA, setB) {
	let _intersection = new Set();
	for (let elem of setB) {
		if (setA.has(elem)) {
			_intersection.add(elem);
		}
	}
	return _intersection;
}
function difference(setA, setB) {
	var _difference = new Set(setA);
	for (var elem of setB) {
		_difference.delete(elem);
	}
	return _difference;
}
function toSet(dict) {
	let t = new Set();
	for (let g in dict) t.add(g[0]);
	return t;
}
function Type(type, name, cmp, meta) {
	name = name || null;
	this.class="type";
	this.type = type;
	this.name = name;
	this.meta = meta;
	cmp = cmp || ((a, b) => true);
	this.cmp = (b) => {
		if(this==b) return true;
		if (type != b.type) return false;
		if (name && b.name) return name == b.name;
		return cmp(meta, b.meta);
	};
	var self = this;
	this.select = (fnObj, fallback) => (fnObj[type] || fallback)(self);
	return this;
}
let dummy = new Type("dummy", "dummy", (a, b) => true);
function Primitive(name) { return new Type("primitive", name); }
function Variant(vmap, name) { return new Type("variant", name, (a, b) => false, vmap); }
function Product(vlis, name) {
	return new Type("product", name, (a, b) => {
		if (a.length != b.length) return false;
		for (let i in a)
			if (!a[i].cmp(b[i]))
				return false;
		return true;
	}, vlis);
}
function bindVariable(name, semi) {
	this.name = name;
	this.semi = semi;
	return this;
}
function Arrow(from, to, polybind, name) {
	return new Type("arrow", name, (a, b) => {
		let [al, ar] = a;
		let [bl, br] = b;
		return al.cmp(bl) && ar.cmp(br);
	}, [from, to, polybind]);
}
function cmpSet(sa, sb) {
	if (sa.size != sb.size) return false;
	for (let a of sa)
		if (!sb.has(a))
			return false;
}
function Bind(semi, varis, bound, free) {
	bound = bound || dummy;
	return new Type("bind", null, (a, b) => {
		if (a[0] != b[0]) return false;
		if (!a[2].cmp(b[2])) return false;
		if (a[3] != b[3]) return false;
		return cmpSet(a[1], b[1]);
	}, [semi, varis, bound, free]);
}
function print(Type) {
	return Type.select({
		"primitive": (x) => x.name,
		"product": (x) => {
			if (x.name) return x.name;
			else return "[" + x.meta.map(print).join(',') + "]";
		},
		"variant": (x) => x.name,
		"arrow": (x) => [x.meta[0], x.meta[1]].map(print).join('->'),
		"bind": (x) => {
			let csum = [];
			for (let c of x.meta[1])
				csum.push(c.name);
			if (!x.meta[2].cmp(dummy))
				csum.push(print(x.meta[2]));
			if (x.meta[3])
				csum.push(x.meta[3].name);
			return x.meta[0].name + "(" + csum.join(',') + ")";
		},
		"dummy": (x) => "dummy"
	});
}
function Semilattice(name) {
	let typeDict = new Map();
	let paths = [];
	let count = 0;
	this.name = name;
	this.__defineGetter__("count", () => count);
	this.dict = typeDict;
	let types = this.types = new Map();
	this.paths = paths;
	let impls = this.impls = [];
	function addType(a) {
		typeDict.set(a, count);
		paths.push([]);
		impls.push(new Map());
		return count++;
	}
	function findType(name, add) {
		add = add || 0;
		if (typeDict.has(name)) return typeDict.get(name);
		if (add) return addType(name);
		throw(new Error("Type not found in typeclass"));
	}
	function add(tyA, tyB) {
		let [a, b] = [findType(tyA.name, 1), findType(tyB.name, 1)];
		types.set(a, tyA);
		types.set(b, tyB);
		paths[a].push(b);
	}
	function addImpl(tyA, tyB, impl) {
		let [a, b] = [findType(tyA.name), findType(tyB.name)];
		impls[a].set(b, impl);
	}
	this.add = (a, b) => add(a, b);
	this.addImpl = (a, b, i) => addImpl(a, b, i);
	let reachSet = [];
	this.reachSet = reachSet;
	function join_id(i, j) {
		let t = intersection(reachSet[i], reachSet[j]);
		if (t.size == 0) return false;
		let max = 0, iset = -1;
		for (let g of t)
			if (reachSet[g].size > max) {
				max = reachSet[g].size;
				iset = g;
			}
		return iset;
	}
	function join(tyA, tyB) {
		let [i, j] = [findType(tyA.name), findType(tyB.name)];
		return join_id(i, j);
	}
	this.find = (a) => findType(a);
	this.join = (a, b) => types.get(join(a, b));
	function check() {
		reachSet.splice(0);
		for (let i = 0; i < count; ++i)
			reachSet.push(new Set());
		function reachAble(i) {
			if (reachSet[i].size)
				return reachSet[i];
			else {
				reachSet[i].add(i);
				for (let t in paths[i])
					reachSet[i] = union(reachSet[i], reachAble(paths[i][t]));
				return reachSet[i];
			}
		}
		for (let i = 0; i < count; ++i)
			reachAble(i);
		for (let i = 0; i < count; ++i)
			for (let j = i + 1; j < count; ++j)
				if (!reachSet[j].has(i) && !reachSet[i].has(j)) {
					g = reachSet[join_id(i, j)];
					if (!g) return false;
					if (!isSuperset(g, intersection(reachSet[i], reachSet[j])))
						return false; /// not semilattice
				} else if (reachSet[i].has(j) && reachSet[j].has(i))
					return false; /// loop detected
		return true;
	}
	this.check = () => check();
	function routeId(i, j) {
		let map = new Map();
		let queue = [];
		function push(i, j) {
			map.set(i, j);
			queue.push(i);
		}
		function has(i) { return map.has(i); }
		push(i);
		while (queue.length) {
			let qr = queue.shift();
			if (qr == j) break;
			for (let i of paths[qr])
				if (!has(i)) push(i, qr);
		}
		if (has(j)) {
			let cret = [];
			function trace(t) {
				if (t == i) return;
				let z = map.get(t);
				trace(z);
				cret.push(impls[z].get(t));
			}
			trace(j);
			return cret;
		} else throw("Type coercion not available");
	}
	this.route = (tyA, tyB) => routeId(findType(tyA.name), findType(tyB.name));
	/*
	 let semi = this;
	 let make = function () {
		 let args = [...arguments];
		 let q = {class:"semi",semi:semi,args:args};
		 q.select =(x) => {return (x[name] || ((x) => x))(q);}
		 return q;
	 };
	 make.__proto__ = this;
	make.__defineGetter__("name",()=>name);*/
	return this;
}
function join(semi, tyA, tyB) {
	if (tyA.cmp(dummy)) return tyB;
	if (tyB.cmp(dummy)) return tyA;
	if (tyA.type == "bind") {
		let [tyAEval, tyABound, tyAFree] = tyA.select({"bind": ((x) => [x.meta[2], x.meta[1], x.meta[3]])}, ((x) => [x, (new Set()), null]));
		let [tyBEval, tyBBound, tyBFree] = tyB.select({"bind": ((x) => [x.meta[2], x.meta[1], x.meta[3]])}, ((x) => [x, (new Set()), null]));
		let [Eval, Bound] = [join(semi, tyAEval, tyBEval), union(tyABound, tyBBound).add(tyAFree).add(tyBFree)];
		if (Bound.has(null)) Bound.delete(null);
		if (Bound.size == 0) return Eval;
		else return Bind(semi, Bound, Eval, null);
	} else if (tyB.type == "bind")
		return join(semi, tyB, tyA);
	else return semi.join(tyA, tyB);
}
function evaluate_bind(tyA, tyB) {
	if (tyA.meta[3] == null) return tyA;
	return join(tyA.meta[0], Bind(tyA.meta[0], tyA.meta[1], tyA.meta[2], null), tyB);
}
function bind(tyA, dict) {
	return tyA.select({
		"product": ((x) => Product(x.meta.map((y) => bind(y, dict)), x.name)),
		"bind": ((x) => {
			let cbound = x.meta[2];
			let vset = new Set(x.meta[1]);
			for (let c of x.meta[1])
				if (dict.has(c)) {
					dict.get(c).select({
						"bind": ((y) => {
							cbound = join(x.meta[0], cbound, y.meta[2]);
							vset.delete(c);
							vset = union(vset, y.meta[1]);
						})
					}, (y) => {
						cbound = join(x.meta[0], cbound, y);
						vset.delete(c);
					});
				}
			if (vset.size == 0 && !x.meta[3])
				return cbound;
			return Bind(x.meta[0], vset, cbound, x.meta[3]);
		}),
		"arrow": ((x) => Arrow(bind(x.meta[0], dict), bind(x.meta[1], dict), difference(x.meta[2], toSet(dict)), x.name))
	}, ((x) => x));
}
function apply(tyA, tyB) {
	if (tyA.type == "bind") {
		let tyApplied = evaluate_bind(tyA, tyB);
		if (!tyApplied.cmp(tyB)) throw("inconsistent types in function applying: A:join(S,{free X})[X->B]!=B");
		return tyApplied;
	} else if (tyA.type == "arrow") {
		let from = tyA.meta[0];
		let to = tyA.meta[1];
		if (from.type == "bind") {
			let tyApplied = evaluate_bind(from, tyB);
			if (!tyApplied.cmp(tyB)) throw("inconsistent types in function applying: A:join(S,{free X})[X->B]!=B");
			if (from.meta[3])
				to = bind(to, (new Map()).set(from.meta[3], tyB));
		} else if (!from.cmp(tyB)) throw("inconsistent types in function applying");
		return to;
	} else throw("none arrow or bind types can't apply");
}
var IntType = new Primitive("Int");
var RationalType = new Primitive("Rat")
var FloatType = new Primitive("Float");
var BooleanType = new Primitive("Boolean");
function Complex(Type) {
	return Product([Type, Type], "Complex" + Type.name);
}
var CI = Complex(IntType);
var CR = Complex(RationalType);
var CF = Complex(FloatType);
var Number = new Semilattice("Number");
Number.add(IntType, RationalType);
Number.add(RationalType, FloatType);
Number.add(IntType,CI);
Number.add(RationalType,CR);
Number.add(FloatType, CF);
Number.add(CI, CR);
Number.add(CR, CF);
Number.check();

var bind1=new bindVariable("A",Number);
var bind2=new bindVariable("B",Number);
var bindType1=Bind(Number,new Set(),dummy,bind1);
var bindType2=Bind(Number,new Set(),dummy,bind2);
var bindType3=Bind(Number,(new Set()).add(bind1).add(bind2),dummy);
var arrowType=Arrow(bindType1,
	Arrow(bindType2,bindType3,(new Set()).add(bind2)),
	(new Set()).add(bind1).add(bind2));

function err(msg,typ,itm){ throw(new Error({msg:msg,typ:typ,itm:itm})); }
function Symbol(name){ this.structure="Symbol";this.name=name;this.type=null;this.value=null;return this; }
function isString(x){ return (typeof x =="string") || (x instanceof String); }
function ToSym(x){ return isString(x)?(new Symbol(x.toString())):x;}
Object.prototype.__defineGetter__("sym",function(){return ToSym(this);});
function If(Cnd,Thn,Els){ return {structure:"If",type:null,value:null,cond:Cnd.sym,then:Thn.sym,else:Els.sym}; }
function Constant(Value,Type){ return {structure:"Constant",value:Value,type:Type}; }
function Call(fn,arg){
	let thisq={};
	thisq.structure="Call";
	thisq.type=null;
	thisq.fn=fn.sym;
	thisq.arg=arg.sym;
	thisq.value=null;
	return thisq;
}
function Let(vari,expr,nxt){
	let thisq={};
	thisq.structure="Let";
	if(vari instanceof Array)
		thisq.bind=vari.map((x)=>x.sym);
	else thisq.bind=vari.sym;
	thisq.to=expr.sym;
	thisq.nxt=nxt.sym;
	thisq.value=null;
	thisq.type=null;
	return thisq;
}
function Match(vari,expr){
	let thisq={};
	thisq.structure="Match";
	thisq.bind=vari.map((x)=>[x[0],x[1].sym,x[2].sym]);
	thisq.to=expr.sym;
	thisq.value=null;
	thisq.type=null;
	return thisq;
}
function TypeCheck(str,dict){
  dict=dict||(new Map());
	return {
		"Symbol":((x)=>{
			if(dict.has(x.name))
				return dict.get(x.name);
			err("name unbound");
		}),
		"If":((x)=>{
			if(!TypeCheck(x.cond,dict).cmp(BooleanType))
				err("condition requires boolean type");
			let ta=TypeCheck(x.then,dict),tb=TypeCheck(x.else,dict);
			if(!ta.cmp(tb))
				err("if branches type mismatch");
			return ta;
		}),
		"Constant":((x)=>x.type),
		"Call":((x)=>{
			let argType=TypeCheck(x.arg,dict);
			let fnType=TypeCheck(x.fn,dict);
			let retType=null;
			try{retType=apply(fnType,argType);}catch(e){err("parameter type mismatch");}
			return retType;
		}),
		"Let":((x)=>{
			let exprType=TypeCheck(x.to,dict);
			let dictr=new Map(dict);
			if(x.bind instanceof Array){
				if(exprType.type!="product")
					err("depacking a type that is not of product type");
				if(x.bind.length != exprType.meta.length)
					err("depack of different lengths");
				for(let i=0;i<x.bind.length;++i)
					dictr.set(x.bind[i].name,exprType.meta[i]);
			}else dictr.set(x.bind.name, exprType);
			return TypeCheck(x.nxt,dictr);
		}),
		"Match":((x)=>{
			let exprType=TypeCheck(x.to,dict);
			let type=null;
			for(let g of x.bind){
				if(!exprType.meta.has(g[0]))
					err("Label not match");
				let dictr=new Map(dict);
				dictr.set(g[1].name,exprType.meta.get(g[0]));
				let ctype=TypeCheck(g[2],dictr);
				if(type && !ctype.cmp(type))
					err("Matching don't obtain same type");
				type=ctype;
			}
			return type;
		})
	}[str.structure](str);
}
var prog1 = If(Constant(true,BooleanType),Constant(1,IntType),Constant(2,IntType));
var prog2 = 
Let("add",Constant(null,arrowType),
  Let(["A","B"],Constant([0,1.],Product([IntType,FloatType])),
    Call("add","A")));

var maybeInt=
  Variant(
    (new Map())
      .set("No",Primitive("Unit"))
      .set("Yes",IntType),
    "maybeInt");
var prog3 =
Let("add",Constant(null,arrowType),
  Let("A",Constant(null,maybeInt),
    Match([
      ["No","A",Constant(0,IntType)],
      ["Yes","A","A"]
    ],"A")));

var prog4=
Let("A","B","A");
