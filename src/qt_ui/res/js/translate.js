var langs = ['en', 'zh'];
var langCode = 'zh';
var langCodeF = '';
var langJS = null;
var trlist;
var translate = function (jsdata) {
    $("[tr]").each(function (index) {
        
        var strTr = jsdata [$(this).attr('tr')];
        if(typeof strTr==="undefined")
            strTr=$(this).attr('tr');
        $(this).html(strTr);

    });
    $("[placeholder]").each(function (index) {
        var strTr = jsdata [$(this).attr('placeholder')];
        $(this).attr("placeholder",strTr);
    });
};

//langCodeF = navigator.language;
//langCode = langCodeF.substr(0, 2);
console.log(navigator.language);
langCodeF = BrowserAPI.getLang();
console.log(langCodeF);
if (langCodeF.userlang)
    langCodeF = langCodeF.userlang;
else
    langCodeF = langCodeF.systemlang;
langCode = langCodeF.substr(0, 2);

var doTranslate = function () {
    var langFile = ($.inArray(langCode, langs) >= 0) ? '../js/lang/' + langCode + '.json' : '../js/lang/en.json';
    console.log(langCode);
    console.log(langFile);
    var t;
    $.getJSON(langFile, translate);
    $.ajax({
        url: langFile,
        async: false,
        dataType: 'json',
        success: function (data) {
            t = data;
        }
    });
    trlist = t;
}
var TR=function(a){
    if(typeof trlist[a]==="undefined")
            return a;
        return trlist[a];
}