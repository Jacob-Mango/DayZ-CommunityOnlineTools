/*
	COT_VehicleManager - marqueur carte

	Le ChangeColor() d'origine de COT ne fait que stocker la couleur sans
	l'appliquer aux widgets. On le corrige pour qu'il recolore reellement
	l'icone et le texte, et on expose la position pour le matching.
*/

modded class JMVehiclesMapMarker
{
	override void ChangeColor( int color )
	{
		m_MarkerColor = color;
		if ( m_Icon )
			m_Icon.SetColor( color );
		if ( m_Name )
			m_Name.SetColor( color );
	}

	vector CVM_GetPos()
	{
		return m_MarkerPos;
	}
}
