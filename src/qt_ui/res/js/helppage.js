$(document).ready(function () {
    $("#tpls").load("templates.html", function () {
        var path = '../json/help_'+langCode+'.json';
        if (CUtil.getGet("lang")) {
            langCode = CUtil.getGet("lang");
            path = "../json/help_" + langCode + ".json";
        }
        CUtil.initGParam();
        doTranslate();
        CPage.prepareNotice("help");
        CPage.updateBalance();
        CPage.updateCblc();
        CPage.registerNotifications();
        CHelp.getJson(path);
        CHelp.renderHelp();
        $("a").click(function () {
            if ($(this).hasClass("goto")) {
                var url = CUtil.setGet("hid", $(this).attr("id"));
                url = CUtil.setGet("lang", langCode, url);
                window.location.href = url;
            }
        });
        $(".goto").hover(function () {
            $(this).css("cursor", "pointer");
        });
        var helpId = CHelp.getHelpId() ? CHelp.getHelpId() : "general";
        CHelp.renderHelpContent(helpId);
        $(".langflag").unbind().click(function () {
            langCodeF = $(this).attr("lang");
            langCode = langCodeF.substr(0, 2);
            doTranslate();
            var path = "../json/help_" + langCode + ".json";
            CHelp.getJson(path);
            CHelp.renderHelp();
            $(this).parent().parent().find(".langflag").removeClass("active");
            $(this).addClass("active");
        });
    });
});

