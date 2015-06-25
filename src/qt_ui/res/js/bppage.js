function copyToClipboard(text) {
    window.prompt("Copy to clipboard: Ctrl+C (Cmd+C for mac), Enter", text);
}
$(document).ready(function () {
    $(".id").find("a.text").click(function () {
//        CBrowser.toggleFullId($(this).parent());
        CBrowser.toggleIdOpt($(this).parent());
    });
    $(".id-share-btn").click(function () {
        alert("To Do");
    });
    $(".brctt").find("a.shrt").click(function () {
        CBrowser.toggleCmt($(this).parent());
    });
//    $(".ctt").click(function () {
//        CBrowser.showFullImg($(this));
//    });
    $(".id-follow-btn").click(function () {
        var id = $(this).parent().parent().find(".text").attr("fullid");
        var feedback = BrowserAPI.setFollow(id);
        for (k in feedback) {
            if (feedback[k] == id) {
                CBrowser.showNotice("Successfully followed " + id);
                return;
            }
        }
        CBrowser.showNotice("Failed");
        console.log(feedback);
    });
    $(".id-unfollow-btn").click(function () {
        var id = $(this).parent().parent().find(".text").attr("fullid");
        var feedback = BrowserAPI.setUnfollow(id);
        if (feedback === true) {
            CBrowser.showNotice("Successfully unfollowed " + id);
            return;
        }
        CBrowser.showNotice("Failed");
        console.log(feedback);
    });
    $(".id-copyid-btn").click(function () {
        var id = $(this).parent().parent().find(".text").attr("fullid");
        copyToClipboard(id);
    });
});
