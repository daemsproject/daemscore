$(document).ready(function () {
    $(".tabbar").children("li").children("a.ntcbtn").click(function () {
        console.log("switchtab");
        CBrowser.switchTab($(this).attr("id"));
    });
    CPublisher.regFeeRate();

});
function prepareSimplePub(type) {
    type = typeof type === "undefined" ? "simple" : type;
    var dropZone = document.getElementById('fileholder');
    var theTextVal = TR('Type some here or drag and drop a file');
    $("#theText").attr("placeholder", theTextVal);
    $('#browsefile').click(function () {
        CPublisher.handleFileInput('theFile', type);
    })
    $('#theText').click(function () {
        $(this).removeClass('lttext');
        $('#pubbtnh').show();
    });
    $('#addtag').click(function () {
        CPublisher.addTagField();
    });
    $('#addlink').click(function () {
        CPublisher.addLinkField();
        $(this).attr('disabled', true);
    });
    if (window.File && window.FileReader && window.FileList && window.Blob) {
        if (dropZone) {
            dropZone.addEventListener('dragover', CPublisher.handleDragOver, false);
            if (type === "simple")
                dropZone.addEventListener('drop', CPublisher.handleFileSelect, false);
            else if (type == "large")
                dropZone.addEventListener('drop', CPublisher.handleFileSelectLarge, false);
            else if (type === "batch")
                dropZone.addEventListener('drop', CPublisher.handleFileSelectBatch, false);
        }
    } else {
        alert("error handleDragOver");
    }
    ;
    $("#theText").bind('input propertychange', function () {
        $("#confirmpub").removeAttr('disabled');
        CPublisher.showDetails();
        CPublisher.refreshFee();
    });
    $("#input-link").bind('input propertychange', function () {
        $("#confirmpub").removeAttr('disabled');
        CPublisher.showDetails();
        CPublisher.refreshFee();
    });
    $("#input-feerate").bind('input propertychange', function () {
        CPublisher.refreshFee();
    });
    $("#promctt-value").bind('input propertychange', function () {
        CPublisher.refreshProTag();
        CPublisher.addDefaultProUntil();
    });
    $("#promctt-check").click(function () {
        $(".input-tag-wrapper").removeClass("hide");
    });
    $("#confirmpub").click(function () {
        var text = $("#theText").val();
        var haveFile = typeof bufferedFile.ctt !== 'undefined';
        var linkstr = $("#input-link").val();
        var inputLname = $("#input-link-name").val();
        if (CUtil.isLinkHttp(linkstr)) {
            var link = {linktype: "HTTP", linkstr: linkstr};
            link.linkname = inputLname.length > 0 ? inputLname : "";
        } else {
            var link = CLink.setString(linkstr, inputLname);
        }
        if (link.linktype === "BLOCKCHAIN") {
            if (!link.isValid() && !link.isEmpty()) {
                CBrowser.showNotice("Link is invalid");
                return;
            }
        }
        var inputlang = CPublisher.getInputLang();
        var tags = CPublisher.getInputTags();
        var tctt = null;
        var lctt = null;
        var fctt = null;
        var tgctt = null;
        var lgctt = null;
        if (text !== "")
            tctt = CPublisher.createTextContent(text);
        if (link.linktype === "HTTP" || (link.linktype === "BLOCKCHAIN" && link.isValid()))
            lctt = CPublisher.createLinkContent(link);
//        console.log(lctt);
        if (haveFile)
            fctt = bufferedFile.ctt;
        if (inputlang)
            lgctt = CPublisher.createLangContent(inputlang);
        if (tags.length > 0)
            tgctt = CPublisher.createTagContent(tags);
        if (!tctt && !lctt && !fctt && tgctt) {
            CBrowser.showNotice(TR('Tags can not be sent without contents!'))
            return;
        }
        var ctt = CPublisher.combineCtt(tctt, lctt, fctt, tgctt, lgctt);
        CPublisher.handleContent(ctt);
    });
    $("#clearpub").click(function () {
        CPublisher.clear();
    });
}

function prepareStdTpl() {
    $(".id").find("a.text").click(function () {
        CBrowser.toggleIdOpt($(this).parent());
    });
    $(".cmt").find("a.shrt").click(function () {
        console.log("cmt");
        CBrowser.toggleCmt($(this).parent());
    });
    $(".id-follow-btn").click(function () {
        var id = $(this).parent().parent().find(".text").attr("fullid");
        var domain = $(this).parent().parent().find(".text").attr("domain");
        var id2fll = domain ? domain : id;
        var feedback = domain ? CBrowser.setFollow(id2fll, "domain") : CBrowser.setFollow(id2fll, "id");
        for (var k in feedback) {
            if (feedback[k] == id2fll) {
                console.log("follow");
                CBrowser.showNotice(TR('Successfully followed ') + id2fll);
                return;
            }
        }
        CBrowser.showNotice("Failed");
        console.log(feedback);
    });
    $(".id-unfollow-btn").click(function () {
        var id = $(this).parent().parent().find(".text").attr("fullid");
        var domain = $(this).parent().parent().find(".text").attr("domain");
        var id2fll = domain ? domain : id;
        var feedback = domain ? CBrowser.setUnfollow(id2fll, "domain") : CBrowser.setUnfollow(id2fll, "id");
        if (feedback === true) {
            CBrowser.showNotice(TR('Successfully unfollowed ') + id2fll);
            return;
        }
        CBrowser.showNotice("Failed");
        console.log(feedback);
    });
    $(".id-copyid-btn").click(function () {
        var id = $(this).parent().parent().find(".text").attr("fullid");
        if (typeof id === "undefined")
            id = $(this).parent().parent().parent().parent().find(".navi-name").attr("fullid");
        CUtil.copyToClipboard(id);
    });
    $(".id-homepage-btn").click(function () {
        var id = $(this).parent().parent().find(".text").attr("fullid");
        var url = "fai:browser/?id=" + id;
        BrowserAPI.goToCustomPage(url);
    });
    $(".id-chat-btn").click(function () {
        var id = $(this).parent().parent().find(".text").attr("fullid");
        var url = "fai:messenger/?chatto=" + id;
        BrowserAPI.goToCustomPage(url);
    });
    $(".ctt-link-btn").click(function () {
        var link = $(this).parent().parent().find(".linkspan").attr("clink");
        CUtil.copyToClipboard(link);
    });
    $(".ctt-share-btn").click(function () {
        var link = $(this).parent().parent().find(".linkspan").attr("clink");
        CPublisher.clearWithAlert();
        CPublisher.shareLink(link);
    });

    $(".ctt-cmt-btn").click(function () {
        var link = $(this).parent().parent().find(".linkspan").attr("clink");
        var id = $(this).parent().parent().parent().parent().parent().find(".id").find(".text").attr("fullid");
        CPublisher.clearWithAlert();
        CPublisher.commentLink(link, id);
    });
    $(".id-share-btn").click(function () {
        var id = $(this).parent().parent().find(".text").attr("fullid");
        var domain = $(this).parent().parent().find(".text").attr("domain");
        var id2share = domain ? domain : id;
        CPublisher.clearWithAlert();
        CPublisher.shareId(id2share);
    });
    $(".prd-pchs-btn").click(function () {
        var link = $(this).parent().parent().find(".text").attr("fullid");
        var url = "fai:shop/?buy=" + link;
        BrowserAPI.goToCustomPage(url);
    });
}


