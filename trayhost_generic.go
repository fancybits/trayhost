// +build linux darwin

package trayhost

import "C"

func addMenuItem(id int, item MenuItem) {
	CAddMenuItem((C.int)(id), C.CString(item.Title), cbool(item.Disabled))
}