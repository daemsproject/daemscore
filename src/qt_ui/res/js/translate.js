var langs = ['en', 'zh'];
var langCode = '';
var langCodeF = '';
var langJS = null;
var TR;
var translate = function (jsdata) {
    $("[tr]").each(function (index) {
        var strTr = jsdata [$(this).attr('tr')];
        $(this).html(strTr);
        
    });
};
langCodeF = navigator.language;
langCode = langCodeF.substr(0, 2);
//console.log(navigator.language);
var doTranslate = function () {
    var langFile = ($.inArray(langCode, langs) >= 0) ? '../js/lang/' + langCode + '.json' : '../js/lang/en.json';
    var t;
    $.getJSON(langFile, translate);
    $.ajax({
    	url: langFile,
    	async: false,
    	dataType: 'json',
    	success: function(data) {
    		t = data;
    	}
    });
    TR = t;
}