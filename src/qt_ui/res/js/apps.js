$(document).ready(function () {
    $("#tpls").load("templates.html", function () {
        CUtil.initGParam();
        doTranslate();
        CPage.prepareNotice("apps");
        CPage.updateBalance();
        CPage.updateCblc();
        CPage.registerNotifications();
    });
});