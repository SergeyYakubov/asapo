//+build !test

package database

import (
	"errors"
	"github.com/globalsign/mgo/bson"
	"github.com/knocknote/vitess-sqlparser/sqlparser"
	"strconv"
)

var global_query bson.M

func SQLOperatorToMongo(sqlOp string) string {
	switch sqlOp {
	case sqlparser.EqualStr:
		return "$eq"
	case sqlparser.LessThanStr:
		return "$lt"
	case sqlparser.GreaterThanStr:
		return "$gt"
	case sqlparser.LessEqualStr:
		return "$lte"
	case sqlparser.GreaterEqualStr:
		return "$gte"
	case sqlparser.NotEqualStr:
		return "$ne"
	case sqlparser.InStr:
		return "$in"
	case sqlparser.NotInStr:
		return "$nin"
		//	case sqlparser.LikeStr:
		//		return "$eq"
		//	case sqlparser.NotLikeStr:
		//		return "$eq"
	case sqlparser.RegexpStr:
		return "$regex"
	case sqlparser.NotRegexpStr:
		return "$regex"
	default:
		return "unknown"
	}
}

func bsonM(key string, val *sqlparser.SQLVal) bson.M {
	switch val.Type {
	case sqlparser.IntVal:
		num, _ := strconv.Atoi(string(val.Val))
		return bson.M{key: num}
	case sqlparser.FloatVal:
		num, _ := strconv.ParseFloat(string(val.Val), 64)
		return bson.M{key: num}
	case sqlparser.StrVal:
		str := string(val.Val)
		return bson.M{key: str}
	default:
		return bson.M{}
	}
}

func bsonMArray(key string, vals []*sqlparser.SQLVal) bson.M {
	if len(vals) == 0 {
		return bson.M{}
	}
	switch vals[0].Type {
	case sqlparser.IntVal:
		nums := make([]int, len(vals))
		for i, val := range vals {
			nums[i], _ = strconv.Atoi(string(val.Val))
		}
		return bson.M{key: nums}
	case sqlparser.FloatVal:
		nums := make([]float64, len(vals))
		for i, val := range vals {
			nums[i], _ = strconv.ParseFloat(string(val.Val), 64)
		}
		return bson.M{key: nums}
	case sqlparser.StrVal:
		strings := make([]string, len(vals))
		for i, val := range vals {
			strings[i] = string(val.Val)
		}
		return bson.M{key: strings}
	default:
		return bson.M{}
	}
}

func keyFromColumnName(cn *sqlparser.ColName) string {
	par_key := cn.Qualifier.Name.String()
	par_par_key := cn.Qualifier.Qualifier.String()
	key := cn.Name.String()
	if len(par_key) > 0 {
		key = par_key + "." + key
	}
	if len(par_par_key) > 0 {
		key = par_par_key + "." + key
	}
	return key
}

func Visit(node sqlparser.SQLNode) (kontinue bool, err error) {
	switch expr := node.(type) {
	case *sqlparser.ComparisonExpr:
		mongoOp := SQLOperatorToMongo(expr.Operator)
		key := keyFromColumnName(expr.Left.(*sqlparser.ColName))
		var vals []*sqlparser.SQLVal
		if tuple, ok := expr.Right.(sqlparser.ValTuple); ok {
			for _, elem := range tuple {
				val, ok := elem.(*sqlparser.SQLVal)
				if !ok {
					return false, errors.New("wrong value")
				}
				vals = append(vals, val)
			}
			global_query = bson.M{key: bsonMArray(mongoOp, vals)}
		} else {
			val, con_err := expr.Right.(*sqlparser.SQLVal)
			if !con_err {
				return false, errors.New("wrong value")
			}
			if expr.Operator == sqlparser.NotRegexpStr {
				global_query = bson.M{key: bson.M{"$not": bsonM(mongoOp, val)}}
			} else {
				global_query = bson.M{key: bsonM(mongoOp, val)}
			}
		}

	case *sqlparser.RangeCond:
		key := keyFromColumnName(expr.Left.(*sqlparser.ColName))
		var mongoOpLeft, mongoOpRight, mongoCond string
		if expr.Operator == sqlparser.BetweenStr {
			mongoOpLeft = "$gte"
			mongoOpRight = "$lte"
			mongoCond = "$and"
		} else {
			mongoOpLeft = "$lt"
			mongoOpRight = "$gt"
			mongoCond = "$or"
		}
		from, con_err := expr.From.(*sqlparser.SQLVal)
		if !con_err {
			return false, errors.New("wrong value")
		}
		to, con_err := expr.To.(*sqlparser.SQLVal)
		if !con_err {
			return false, errors.New("wrong value")
		}
		global_query = bson.M{mongoCond: []bson.M{{key: bsonM(mongoOpLeft, from)},
			{key: bsonM(mongoOpRight, to)}},
		}
	default:
		return false, errors.New("unkwnown expression ")
	}

	return false, nil
}

func (db *Mongodb) BSONFromSQL(dbname string, query string) (bson.M, error) {
	global_query = bson.M{}

	stmt, err := sqlparser.Parse("select * from " + dbname + " where " + query)
	if err != nil {
		return global_query, err
	}

	switch stmt := stmt.(type) {
	case *sqlparser.Select:
		where := stmt.Where.Expr
		err = sqlparser.Walk(Visit, where)
	}
	return global_query, err
}
