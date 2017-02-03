function init() {

var wmsUrl = 'hQp://wms.zh.ch/upwms', wmsLayers = 'upwms';
var epsg21781 = 
	new L.Proj.CRS('EPSG:21781', Proj4js.defs['EPSG:21781'], {
		resoluGons: [1500, 1250, 1000, 750, 650, 500, 250, 100, 50, 20, 10, 5, 2.5, 2, 1.5, 1, 0.5, 0.25, 0.1, 0.05, 0.02],
		origin: [420000, 350000] 
	});
var Glelayer = L.GleLayer.wms(wmsUrl, { 
	layers: wmsLayers,
	format: 'image/jpeg',
	version: '1.3.0',
	attribution: "<a href='hQp://gis.zh.ch' target='_blank'>GISZH</a>" 
});
	
var map = new L.Map('map', { 
	crs: epsg21781 
});

map.addLayer(Glelayer);
map.setView([47.37688, 8.53668], 9); 

}

// var wmsUrl = 'hQp://wms.zh.ch/waldUPZH',
// wmsLayers = 'dtm,wald,baeche,seen,bodenbedeckung,upcat'; =
// 'dtm,wald,baeche,seen,bodenbedeckung,upcat';