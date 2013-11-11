function CaptchaReload(id, src) {
    var d = new Date();
    $("#"+id).attr("src", src+"?"+d.getTime());
}
