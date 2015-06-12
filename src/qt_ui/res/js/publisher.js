var CPublisher = new function () {
    var CPublisher = this;

    this.handleFiles = function (files) {
        for (var i = 0, f; f = files[i]; i++) {
            if (f.size > 1000000) {
                CBrowser.showNotice("The maximum file size is 10MB");
                return;
            }
            if (f.name) {
                var r = new FileReader();
//                r.readAsText(f);
                r.readAsDataURL(f);
                r.onload = (function (f) {
                    return function (e) {
                        var raw = e.target.result;
                        var rctt = CUtil.decodeDataUrl(raw);
                        rctt["filename"] = f.name;
                        console.log("rctt ");
                        console.log(rctt);

                        var ctthex = BrowserAPI.createContentC(rctt.type, rctt);
                        var start = new Date().getTime();
                        console.log(start);
                        console.log(ctthex.hex.substr(0, 20) + "...(" + ctthex.hex.length / 2 + " bytes)");
                        var end = new Date().getTime();
                        console.log(end);
                        var ctt = BrowserAPI.getContentByString(ctthex.hex);
                        ctt.poster = [BrowserAPI.getAccountID()];
                        CPublisher.handleContent(ctt, ctthex.hex);
                    };
                })(f);
            }
        }
    };
    this.handleContent = function (ctt, hexctt) {
        ctt.link = "";
        console.log(ctt);
        if (!CBrowser.addContent(ctt, CONTENT_TYPE_MINE, false))
            return false;
        $("#confirmpub").removeAttr('disabled');
        var rawtx = BrowserAPI.createTxByContent(hexctt);
        console.log(rawtx);
    };
    this.handleDragOver = function (evt) {
        evt.stopPropagation();
        evt.preventDefault();
        evt.dataTransfer.dropEffect = 'copy'; // Explicitly show this is a copy.
    };
    this.handleFileInput = function (elemId) {
        var elem = document.getElementById(elemId);
        if (elem && document.createEvent) {
            var evt = document.createEvent("MouseEvents");
            evt.initEvent("click", true, false);
            elem.dispatchEvent(evt);
        }

        input = document.getElementById('theFile');
        this.handleFiles(input.files);
    };
    this.handleFileSelect = function (evt) {
        evt.stopPropagation();
        evt.preventDefault();
        CPublisher.handleFiles(evt.dataTransfer.files);
        return;
    };
};

$(document).ready(function () {
    var dropZone = document.getElementById('fileholder');
    $('#fileholder').click(function () {
        CPublisher.handleFileInput('theFile');
    })
    if (window.File && window.FileReader && window.FileList && window.Blob) {
        dropZone.addEventListener('dragover', CPublisher.handleDragOver, false);
        dropZone.addEventListener('drop', CPublisher.handleFileSelect, false);
    } else {
        alert("error handleDragOver");
    }
    $("#test").click(function () {
        var rctt = "test text";
        var ctt = BrowserAPI.createContentC(rctt);
        console.log(ctt.hex);
    });
    $(".id").find("a.text").click(function () {
        CBrowser.toggleFullId($(this).parent());
    });
});



