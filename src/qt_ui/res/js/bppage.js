$(document).ready(function () {
    $(".tabbar").children("li").children("a.ntcbtn").unbind().click(function () {
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
    $('#browsefile').unbind().click(function () {
        CPublisher.handleFileInput('theFile', type);
    })
    $('#theText').unbind().click(function () {
        $(this).removeClass('lttext');
        $('#pubbtnh').show();
    });
    $('#addtag').unbind().click(function () {
        CPublisher.addTagField();
    });
    $('#addlink').unbind().click(function () {
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
        CPublisher.refreshPromTag();
        CPublisher.addDefaultProUntil();
    }).keyup(function () {
        $(this).val(this.value.replace(/[^0-9\.]+/g, ''));
    });
    $("#promctt-check").unbind().click(function () {
        $(".input-tag-wrapper").removeClass("hide");
    });
    $("#confirmpub").unbind().click(function () {
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
                CPage.showNotice("Link is invalid");
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
            CPage.showNotice(TR('Tags can not be sent without contents!'))
            return;
        }
        var ctt = CPublisher.combineCtt(tctt, lctt, fctt, tgctt, lgctt);
        CPublisher.handleContent(ctt);
    });
    $("#clearpub").unbind().click(function () {
        CPublisher.clear();
    });

}