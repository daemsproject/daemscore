var HELP = {};

$(document).ready(function () {

    $("#tpls").load("templates.html", function () {
        CUtil.initGParam();
        doTranslate();
        CPage.prepareNotice("help");
        CPage.updateBalance();
        CPage.updateCblc();
        CPage.registerNotifications();
        CHelp.getJson();
        CHelp.renderHelp();
        $("a").click(function () {
            if ($(this).hasClass("goto")) {
                var url = CUtil.setGet("hid", $(this).attr("id"));
                window.location.href = url;
            }
        });
        $(".goto").hover(function () {
            $(this).css("cursor", "pointer");
        });
        var helpId = CHelp.getHelpId();
        if (helpId)
            CHelp.renderHelpContent(helpId);

    });
});

