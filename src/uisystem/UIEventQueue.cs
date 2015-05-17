using System.Collections.Generic;

namespace CCGUI
{
	public class UIEventQueue : EventHandler
	{
		private Dictionary<string, int> m_events = new Dictionary<string, int>();
		
		public UIEventQueue()
		{
			
		}
		
		public void Clear()
		{
			m_events.Clear();
		}
		
		public void OnEvent(string name)
		{
			if (m_events.ContainsKey(name))
				m_events[name]++;
			else
				m_events[name] = 1;
		}
		
		// See if this happened? EventHandler must store somehow...
		public bool PollEvent(string name)
		{
			if (!m_events.ContainsKey(name))
				return false;
			
			if (m_events[name] > 1)
			{
				m_events[name]++;
				return true;
			}
			else
			{
				m_events.Remove(name);
				return true;
			}			
		}
		
		public ICollection<string> PeekAllEvents()
		{
			return m_events.Keys;
		}	
	}
}