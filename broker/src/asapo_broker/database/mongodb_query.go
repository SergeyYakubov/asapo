//+build !test

package database

import (
	"errors"
	"fmt"
	"github.com/globalsign/mgo/bson"
	"github.com/knocknote/vitess-sqlparser/sqlparser"
	"strconv"
)

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

func processComparisonExpr(expr *sqlparser.ComparisonExpr) (res bson.M, err error) {
	mongoOp := SQLOperatorToMongo(expr.Operator)
	key := keyFromColumnName(expr.Left.(*sqlparser.ColName))
	var vals []*sqlparser.SQLVal
	if tuple, ok := expr.Right.(sqlparser.ValTuple); ok { // SQL in
		for _, elem := range tuple {
			val, ok := elem.(*sqlparser.SQLVal)
			if !ok {
				return bson.M{}, errors.New("wrong value")
			}
			vals = append(vals, val)
		}
		return bson.M{key: bsonMArray(mongoOp, vals)}, nil
	} else { // SQL =,>,<,>=,<=,regexp
		val, ok := expr.Right.(*sqlparser.SQLVal)
		if !ok {
			return bson.M{}, errors.New("wrong value")
		}
		if expr.Operator == sqlparser.NotRegexpStr {
			return bson.M{key: bson.M{"$not": bsonM(mongoOp, val)}}, nil
		} else {
			return bson.M{key: bsonM(mongoOp, val)}, nil
		}
	}
}

func processRangeCond(expr *sqlparser.RangeCond) (res bson.M, err error) {
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
	from, ok := expr.From.(*sqlparser.SQLVal)
	if !ok {
		return bson.M{}, errors.New("wrong value")
	}
	to, ok := expr.To.(*sqlparser.SQLVal)
	if !ok {
		return bson.M{}, errors.New("wrong value")
	}
	return bson.M{mongoCond: []bson.M{{key: bsonM(mongoOpLeft, from)},
		{key: bsonM(mongoOpRight, to)}}}, nil

}

func processAndOrExpression(left sqlparser.Expr, right sqlparser.Expr, op string) (res bson.M, err error) {
	bson_left, errLeft := getBSONFromExpression(left)
	if errLeft != nil {
		return bson.M{}, errLeft
	}
	bson_right, errRight := getBSONFromExpression(right)
	if errRight != nil {
		return bson.M{}, errRight
	}
	return bson.M{op: []bson.M{bson_left, bson_right}}, nil
}

func getBSONFromExpression(node sqlparser.Expr) (res bson.M, err error) {
	switch expr := node.(type) {
	case *sqlparser.ComparisonExpr:
		return processComparisonExpr(expr)
	case *sqlparser.RangeCond:
		return processRangeCond(expr)
	case *sqlparser.AndExpr:
		return processAndOrExpression(expr.Left, expr.Right, "$and")
	case *sqlparser.OrExpr:
		return processAndOrExpression(expr.Left, expr.Right, "$or")
	case *sqlparser.ParenExpr:
		return getBSONFromExpression(expr.Expr)
	default:
		return bson.M{}, errors.New("unkwnown expression " + fmt.Sprintf("%T", expr))
	}
}

func getSortBSONFromOrderArray(order_array sqlparser.OrderBy) (bson.M, error) {
	if len(order_array) != 1 {
		return bson.M{}, errors.New("order by should have single column name")
	}

	order := order_array[0]
	val, ok := order.Expr.(*sqlparser.ColName)
	if !ok {
		return bson.M{}, errors.New("order has to be key name")
	}

	name := keyFromColumnName(val)
	sign := 1
	if order.Direction == sqlparser.DescScr {
		sign = -1
	}
	return bson.M{name: sign}, nil
}

func (db *Mongodb) BSONFromSQL(dbname string, query string) (bson.M, bson.M, error) {
	stmt, err := sqlparser.Parse("select * from " + dbname + " where " + query)
	if err != nil {
		return bson.M{}, bson.M{}, err
	}

	sel, _ := stmt.(*sqlparser.Select)
	query_mongo, err := getBSONFromExpression(sel.Where.Expr)
	if err != nil || len(sel.OrderBy) == 0 {
		return query_mongo, bson.M{}, err
	}

	sort_mongo, err := getSortBSONFromOrderArray(sel.OrderBy)
	if err != nil {
		return bson.M{}, bson.M{}, err
	}

	return query_mongo, sort_mongo, nil

}
