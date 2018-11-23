#include <vector>
#include "spatRaster.h"
#include "recycle.h"



SpatRaster SpatRaster::warp(SpatRaster x, std::string method, SpatOptions opt) {

	unsigned nl = nlyr();
	SpatRaster out = x.geometry(nl);
	out.setNames(getNames());
	std::vector<std::string> f {"bilinear", "neighbor"};
	if (std::find(f.begin(), f.end(), method) == f.end()) {
		out.setError("unknown warp method");
		return out;
	}
	if (!hasValues()) {
		return out;
	}
	if (compare_geom(x, true, false)) {
		return(*this); // but should be deep copy
	}
	out.extent.intersect(extent);
    if (!out.extent.valid()) {
        setError("No spatial overlap");
        return out;
    }

	unsigned xq = x.xres() / xres();
	unsigned yq = x.yres() / yres();
	if (std::max(xq, yq) > 1) {
		xq = xq == 0 ? 1 : xq;
		yq = yq == 0 ? 1 : yq;
		std::vector<unsigned> agf = {yq, xq, 1};
		SpatRaster a;
		SpatOptions agopt;
		if (method == "bilinear") {
			a = aggregate(agf, "mean", true, agopt);
		} else {
			a = aggregate(agf, "modal", true, agopt);
		}
		source = a.source;
	}
	readStart();
  	out.writeStart(opt);
	std::vector<double> v;
	unsigned firstcell, lastcell;
	std::vector<std::vector<double> > xy;
	for (size_t i = 0; i < out.bs.n; i++) {
        firstcell = out.cellFromRowCol(out.bs.row[i], 0);
		lastcell =  out.cellFromRowCol(out.bs.row[i]+out.bs.nrows[i]-1, out.ncol()-1);
		std::vector<double> cells(1+lastcell-firstcell);
		std::iota (std::begin(cells), std::end(cells), firstcell);
		xy = out.xyFromCell(cells);
		v = extractXY(xy[0], xy[1], method);
		out.writeValues(v, out.bs.row[i]);
	}
	out.writeStop();
	readStop();
	return(out);
}


