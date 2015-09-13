$(document).ready(function () {
    $("#tpls").load("templates.html", function () {
        CUtil.initGParam();
        doTranslate();
        CPage.prepareNotice("downloader");
        CPage.updateBalance();
        CPage.updateCblc();
        CPage.registerNotifications();
    });
});