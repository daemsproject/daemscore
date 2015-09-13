var langs = ['en', 'zh'];
var langCode = 'zh';
var langCodeF = '';
var langJS = null;
var trlist;
var translate = function (jsdata) {
    $("[tr]").each(function (index) {

        var strTr = jsdata [$(this).attr('tr')];
        if (typeof strTr === "undefined")
            strTr = $(this).attr('tr');
        $(this).html(strTr);

    });
    $("[placeholder]").each(function (index) {
        var strTr = jsdata [$(this).attr('placeholder')];
        $(this).attr("placeholder", strTr);
    });
    $("[title]").each(function (index) {
        var strTr = jsdata [$(this).attr('title')];
        $(this).attr("title", strTr);
    });
};
langCodeF = typeof FAI_API === "undefined" ? navigator.language || navigator.userLanguage : FAI_API.getLang();
if (langCodeF.userlang)
    langCodeF = langCodeF.userlang;
else if (langCodeF.systemlang)
    langCodeF = langCodeF.systemlang;
langCode = langCodeF.substr(0, 2);

var doTranslate = function () {
    var langFile = ($.inArray(langCode, langs) >= 0) ? '../js/lang/' + langCode + '.json' : '../js/lang/en.json';
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
var TR = function (a) {
    if (typeof trlist[a] === "undefined")
        return a;
    return trlist[a];
}