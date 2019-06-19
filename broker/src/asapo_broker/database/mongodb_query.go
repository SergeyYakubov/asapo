//+build !test

package database

import (
	"fmt"
	"github.com/globalsign/mgo/bson"
	"github.com/knocknote/vitess-sqlparser/sqlparser"
	"strconv"
)

var res bson.M

func Visit(node sqlparser.SQLNode) (kontinue bool, err error) {
	//fmt.Printf("%T\n", node)
	switch expr := node.(type) {
	case *sqlparser.ComparisonExpr:
		//op := expr.Operator
		key := expr.Left.(*sqlparser.ColName).Name.String()
		val := expr.Right.(*sqlparser.SQLVal)
		if val.Type == sqlparser.IntVal {
			num, _ := strconv.Atoi(string(val.Val))
			res = bson.M{key: num}
		}
	}

	return false, nil
}

func (db *Mongodb) BSONFromSQL(dbname string, query string) (bson.M, error) {
	stmt, err := sqlparser.Parse("select * from " + dbname + " where " + query)
	if err != nil {
		panic(err)
	}

	res = bson.M{}
	switch stmt := stmt.(type) {
	case *sqlparser.Select:
		where := stmt.Where.Expr
		sqlparser.Walk(Visit, where)
	}

	//res = bson.M{"temp": 10}
	fmt.Println(res)

	return res, nil
}
