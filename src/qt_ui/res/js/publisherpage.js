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
    $("#theText").bind('input propertychange', function () {
        $("#confirmpub").removeAttr('disabled');
    });
    $("#confirmpub").click(function () {
        var text = $("#theText").val();
        console.log(text);
        CPublisher.handleText(text);
    });
});