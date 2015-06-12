
var blkDisp;
var fllDisp;
$(document).ready(function () {

    CBrowser.newAction();

    $("#refresh-btn").click(function () {
        CBrowser.refreshNew();
    });
    $("#refreshold-btn").click(function () {
        CBrowser.refreshOld();
    });
    $(".tabbar").children("li").children("a").click(function () {
        CBrowser.switchTab($(this).attr("id"));
    });
    $(".id").find("a.text").click(function () {
        CBrowser.toggleFullId($(this).parent());
    });
    $(".id-follow-btn").click(function () {
        var id = $(this).parent().parent().find(".text").attr("fullid");
        var feedback = BrowserAPI.setFollow(id);
        for (k in feedback) {
            if (feedback[k] == id) {
                CBrowser.showNotice("Successful");
                return;
            }
        }
        CBrowser.showNotice("Failed");
        console.log(feedback);
    });
    $(".id-share-btn").click(function () {
        alert("To Do");
    });
    $(".brctt").find("a.shrt").click(function () {
        CBrowser.toggleCmt($(this).parent());
    });
    $(".ctt").click(function () {
        CBrowser.showFullImg($(this));
    });
    $(".ctt-link-btn").click(function () {
        CBrowser.toggleLink($(this));
    });


    $("#fullImage").click(function () {
        $(this).html("");
    });
    $("#test-btn").click(function () {
        console.log(BrowserAPI.getFollowed());
    });
    $(window).scroll(function () {
        if ($(window).scrollTop() + $(window).height() == $(document).height()) {
            CBrowser.bottomAction();
        }
    });
});
