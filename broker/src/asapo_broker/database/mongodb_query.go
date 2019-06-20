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
		//	case sqlparser.RegexpStr:
		//		return "$eq"
		//	case sqlparser.NotRegexpStr:
		//		return "$eq"
	default:
		return "unknown"
	}
}

/*func bsonM(key string, value interface{}) bson.M {
	v := reflect.ValueOf(value)
	switch v.Kind() {
	case reflect.Int, reflect.Int8, reflect.Int16, reflect.Int32, reflect.Int64:
		return bson.M{key: v.Int()}
	case reflect.Float32, reflect.Float64:
		return bson.M{key: v.Float()}
	case reflect.String:
		return bson.M{key: v.String()}
	case reflect.Bool:
		return bson.M{key: v.Bool()}
	default:
		return bson.M{}
	}
}*/

func bsonM(key string, val *sqlparser.SQLVal) bson.M {
	switch val.Type {
	case sqlparser.IntVal:
		num, _ := strconv.Atoi(string(val.Val))
		return bson.M{key: num}
	case sqlparser.FloatVal:
		num, _ := strconv.ParseFloat(string(val.Val), 64)
		return bson.M{key: num}
	case sqlparser.StrVal:
		return bson.M{key: string(val.Val)}
	default:
		return bson.M{}
	}
}

func Visit(node sqlparser.SQLNode) (kontinue bool, err error) {
	//fmt.Printf("%T\n", node)
	switch expr := node.(type) {
	case *sqlparser.ComparisonExpr:
		mongoOp := SQLOperatorToMongo(expr.Operator)
		par_key := expr.Left.(*sqlparser.ColName).Qualifier.Name.String()
		par_par_key := expr.Left.(*sqlparser.ColName).Qualifier.Qualifier.String()
		key := expr.Left.(*sqlparser.ColName).Name.String()
		if len(par_key) > 0 {
			key = par_key + "." + key
		}
		if len(par_par_key) > 0 {
			key = par_par_key + "." + key
		}
		val, con_err := expr.Right.(*sqlparser.SQLVal)
		if !con_err {
			return false, errors.New("wrong value")
		}

		global_query = bson.M{key: bsonM(mongoOp, val)}
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
