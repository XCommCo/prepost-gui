CONFIG(debug, debug|release) {
	# gdal
	LIBS += -L"E:/iricdev_2013/lib/src/gdal-1.11.2"

	# vtk
	LIBS += -L"E:/iricdev_2013/lib/install/vtk-6.1.0/debug/lib"

	# cgnslib
	LIBS += -L"E:/iricdev_2013/lib/install/cgnslib-3.2.1/debug/lib"

	# iriclib
	LIBS += -L"E:/iricdev_2013/src/iriclib/debug"

	# Qwt
	LIBS += -L"E:/iricdev_2013/lib/install/Qwt-6.1.0/lib"

	# proj.4
	LIBS += -L"E:/iricdev_2013/lib/install/proj-4.8.0/release/lib"

	# shapefile
	LIBS += -L"E:/iricdev_2013/lib/install/shapelib-1.3.0/debug"

	# hdf5
	LIBS += -L"E:/iricdev_2013/lib/install/hdf5-1.8.13/debug/lib"

	# netcdf
	LIBS += -L"E:/iricdev_2013/lib/install/netcdf-4.3.2/debug/lib"
}
else {
	# gdal
	LIBS += -L"E:/iricdev_2013/lib/install/gdal-1.11.2/release/lib"

	# vtk
	LIBS += -L"E:/iricdev_2013/lib/install/vtk-6.1.0/release/lib"

	# cgnslib
	LIBS += -L"E:/iricdev_2013/lib/install/cgnslib-3.2.1/release/lib"

	# iriclib
	LIBS += -L"E:/iricdev_2013/src/iriclib/release"

	# Qwt
	LIBS += -L"E:/iricdev_2013/lib/install/Qwt-6.1.0/lib"

	# proj.4
	LIBS += -L"E:/iricdev_2013/lib/install/proj-4.8.0/release/lib"

	# shapefile
	LIBS += -L"E:/iricdev_2013/lib/install/shapelib-1.3.0/release"

	# hdf5
	LIBS += -L"E:/iricdev_2013/lib/install/hdf5-1.8.13/release/lib"

	# netcdf
	LIBS += -L"E:/iricdev_2013/lib/install/netcdf-4.3.2/release/lib"
}

INCLUDEPATH += .

# gdal
INCLUDEPATH += "E:/iricdev_2013/lib/install/gdal-1.11.2/debug/include"

# vtk
INCLUDEPATH += "E:/iricdev_2013/lib/install/vtk-6.1.0/debug/include/vtk-6.1"

# cgnslib
INCLUDEPATH += "E:/iricdev_2013/lib/install/cgnslib-3.2.1/debug/include"

# iriclib
INCLUDEPATH += "E:/iricdev_2013/src/iriclib"

# Qwt
INCLUDEPATH += "E:/iricdev_2013/lib/install/Qwt-6.1.0/include"

# shapelib
INCLUDEPATH += "E:/iricdev_2013/lib/install/shapelib-1.3.0/release"

# proj.4
INCLUDEPATH += "E:/iricdev_2013/lib/install/proj-4.8.0/release/include"

# netcdf
INCLUDEPATH += "E:/iricdev_2013/lib/install//netcdf-4.3.2/release/include"

#internal libs
INCLUDEPATH += "E:/iricdev_2013/src/libs"
