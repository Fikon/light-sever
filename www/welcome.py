#!/usr/bin/python
# -*- coding: UTF-8 -*-

# filename：test.py

# CGI处理模块
import cgi, cgitb
import sys

# 创建 FieldStorage 的实例化
form = cgi.FieldStorage()



# 获取数据
name = form.getvalue('name')



print "<html>"
print "<head>"
print "<meta charset=\"utf-8\">"
print "<title>Welcome to my server.</title>"
print "</head>"
print "<body>"
print "<h2>Hello, %s ,now I know your name!</h2>" % (name)
print "</body>"
print "</html>"
