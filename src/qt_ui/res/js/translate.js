var langs = ['en', 'zh'];
var langCode = '';
var langCodeF = '';
var langJS = null;
var translate = function (jsdata) {
    $("[tr]").each(function (index) {
        var strTr = jsdata [$(this).attr('tr')];
        $(this).html(strTr);
    });
};
langCodeF = navigator.language;
langCode = langCodeF.substr(0, 2);
console.log(navigator.language);
var doTranslate = function () {
    var langFile = ($.inArray(langCode, langs) >= 0) ? '../js/lang/' + langCode + '.json' : '../js/lang/en.json';
    $.getJSON(langFile, translate);
}