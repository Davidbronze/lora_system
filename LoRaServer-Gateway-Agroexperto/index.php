
<?php
if(isset($_GET["Setor 1"])){
	echo "setor 1 foi ligado";
}
?>

<!DOCTYPE html>

<head>
		<meta charset="utf-8">
		<meta name="viewport" content="width=device-width, initial-scale=1">
		<link rel="stylesheet" type="text/css" href="estilo.css">
		<link href="https://fonts.googleapis.com/css?family=Prompt&display=swap" rel="stylesheet">
		<link rel="shortcut icon" href="imagens/agro3.ico" type="image/vnd.microsoft.icon">
		<title>AgroexPerto</title>
	</head>

<body>
	
<svg width="800" height="600" xmlns="http://www.w3.org/2000/svg">
 <g>

	  <title>Layer 1</title>
	  <rect fill="#fff" stroke="#000" opacity="undefined" x="96.5" y="64.5" width="307" height="482" id="svg_1"/>
	  <rect fill="#A0D58A" stroke="#000" opacity="undefined" x="97.5" y="516.5" width="304" height="28" id="svg_2"/>
	  <rect fill="#A0D58A" stroke="#000" opacity="undefined" x="99.5" y="67.5" width="25" height="89" id="svg_3"/>
	  <rect fill="#A0D58A" stroke="#000" opacity="undefined" x="222.5" y="43.5" width="180" height="18" id="svg_4"/>
	  <rect fill="#A0D58A" stroke="#000" opacity="undefined" x="220.5" y="74.5" width="182" height="18" id="svg_5"/>
	  <rect fill="#A0D58A" stroke="#000" opacity="undefined" x="233.5" y="137.5" width="124" height="20" id="svg_6"/>
	  <rect fill="#A0D58A" stroke="#000" opacity="undefined" x="99.5" y="388.5" width="22" height="124" id="svg_7"/>
	  <rect fill="#A0D58A" stroke="#000" opacity="undefined" x="296.5" y="455.5" width="102" height="57" id="svg_8"/>  
	  <rect fill="#A0D58A" stroke="#000" stroke-width="null" stroke-dasharray="null" stroke-opacity="null" opacity="undefined" x="306.5" y="348.5" width="92" height="14" id="svg_12"/>
	  <path id="svg_16" d="m246.68252,469.79091l12.30145,-13.76591l17.39709,0l12.30145,13.76591l0,19.46818l-12.30145,13.76591l-17.39709,0l-12.30145,-13.76591l0,-19.46818z" opacity="undefined" fill-opacity="null" stroke-opacity="null" stroke-dasharray="null" stroke-width="null" stroke="#000" fill="#396849"/>
	  <text xml:space="preserve" text-anchor="start" font-family="sans-serif" font-size="24" id="svg_9" y="540" x="207.5" opacity="undefined" stroke-width="0" stroke="#000" fill="#000000">desligado</text>
  <text stroke="#000" transform="rotate(-90 108.85665893554689,449.0285339355469) matrix(0.92643678188324,0,0,0.7740734793264216,3.2283820509910592,109.1921318195877) " xml:space="preserve" text-anchor="start" font-family="sans-serif" font-size="24" id="svg_10" y="447.47" x="62.65881" opacity="undefined" stroke-width="0" fill="#000000">desligado</text>
  <text stroke="#000" transform="matrix(0.8832116723060608,0,0,1,46.51095150411129,0) " xml:space="preserve" text-anchor="start" font-family="sans-serif" font-size="24" id="svg_13" y="489" x="287.57438" opacity="undefined" stroke-width="0" fill="#000000">desligado</text>
  <text stroke="#000" transform="matrix(0.825741256738759,0,0,0.7618286402714851,60.51432839819081,84.6879495032583) " xml:space="preserve" text-anchor="start" font-family="sans-serif" font-size="24" id="svg_14" y="362" x="304.5" opacity="undefined" stroke-width="0" fill="#000000">desligado</text>
  <text stroke="#000" transform="matrix(0.499602512220477,0,0,1,127.04357946581047,0) " xml:space="preserve" text-anchor="start" font-family="sans-serif" font-size="24" id="svg_15" y="480" x="230.5" opacity="undefined" stroke-width="0" fill="#000000">desligado</text>

 </g>

</svg>

<form action="" method="get">
		<input type="button" value = "Setor_1_ON" id= "botao1"/>
		<input type="button" value = "Setor_2_ON" id= "botao2"/>
		<input type="button" value = "Setor_3_ON" id= "botao3"/>
		<input type="button" value = "Setor_4_ON" id= "botao4"/>
	</form>

	<form action="" method="get">
		<input type="button" value = "Setor_1_OFF" id= "botao5"/>
		<input type="button" value = "Setor_2_OFF" id= "botao6"/>
		<input type="button" value = "Setor_3_OFF" id= "botao7"/>
		<input type="button" value = "Setor_4_OFF" id= "botao8"/>
	</form>


</body>

<script>// referencia o 'input' do layout html
        
     let buttons = document.querySelectorAll("input");
          
		buttons.forEach((btn, index)=>{
			btn.addEventListener("click", e => {
			let textBtn = btn.value.replace("Setor_","");
			console.log(textBtn);
			switch (textBtn) {
				case '1_ON':
					(document.querySelector("#svg_2")).style.fill= '#00FFFF';				
				break;

				case '2_ON':
					(document.querySelector("#svg_8")).style.fill= '#00FFFF';
				break;

				case '3_ON':
					(document.querySelector("#svg_16")).style.fill= '#00FFFF';
				break;

				case '4_ON':
					(document.querySelector("#svg_12")).style.fill= '#00FFFF';
				break;

				case '1_OFF':
					(document.querySelector("#svg_2")).style.fill= '#A0D58A';				
				break;

				case '2_OFF':
					(document.querySelector("#svg_8")).style.fill= '#A0D58A';
				break;

				case '3_OFF':
					(document.querySelector("#svg_16")).style.fill= '#A0D58A';
				break;

				case '4_OFF':
					(document.querySelector("#svg_12")).style.fill= '#A0D58A';
				break;

				default:
					(document.querySelector("#svg_1")).style.fill = '#A0D58A';;
				break;
			}			
			
			});
		});



</script>